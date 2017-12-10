#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Network.hpp"


namespace ProtoMesh::communication {

    Datagrams Network::processAdvertisement(const Datagram &datagram) {
        using namespace Routing::IARP;

        /// Deserialize the advertisement
        auto advertisementResult = Advertisement::fromBuffer(datagram);
        if (advertisementResult.isErr()) return {};
        Advertisement advertisement = advertisementResult.unwrap();

        /// Discard the advertisement if it originated from us
        if (advertisement.uuid == this->deviceID) return {};

        /// Store the route in the routing table and add ourselves to the list
        this->routingTable.processAdvertisement(advertisement);
        advertisement.addHop(this->deviceID);

        /// Store the key in the credentials store
        this->credentials.insertKey(advertisement.uuid, advertisement.pubKey);

        /// Check if it has already travelled to the border of the zone
        if (advertisement.route.size() > ZONE_RADIUS) return {};

        // TODO Don't rebroadcast to the original sender. Some form of target node white/blacklist maybe
        //      Optionally obviously since not all transmission media might support it.
        return { {MessageTarget::broadcast(), advertisement.serialize()} };
    }

    Datagrams Network::dispatchRouteDiscoveryAcknowledgement(Routing::IERP::RouteDiscovery routeDiscovery) {
        cryptography::UUID firstBorder = routeDiscovery.route.back();
        auto routeToFirstBorderResult = this->routingTable.getRouteTo(firstBorder);
        auto firstBorderPublicKey = this->credentials.getKey(firstBorder);

        if (routeToFirstBorderResult.isOk() && firstBorderPublicKey.isOk()) {
            // TODO Check if the reversing works as intended
            vector<cryptography::UUID> reversedRoute(routeDiscovery.route.end(), routeDiscovery.route.begin());
            this->routeCache.addRoute(routeDiscovery.route.front(), reversedRoute);

            using namespace scheme::communication::ierp;
            flatbuffers::FlatBufferBuilder builder;

            /// Serialize the route
            vector<scheme::cryptography::UUID> routeEntries;
            for (auto hop : routeDiscovery.route)
                routeEntries.push_back(hop.toScheme());

            auto routeVector = builder.CreateVectorOfStructs(routeEntries);

            /// Serialize our public key
            auto pubKey = this->deviceKeys.pub.toBuffer(&builder);

            auto routeDiscoveryAcknowledgement = CreateRouteDiscoveryAcknowledgementDatagram(builder, routeVector,
                                                                                             pubKey);

            /// Convert it to a byte array
            builder.Finish(routeDiscoveryAcknowledgement, RouteDiscoveryAcknowledgementDatagramIdentifier());
            uint8_t *buf = builder.GetBufferPointer();
            vector<uint8_t> routeDiscoveryAcknowledgementDatagram({buf, buf + builder.GetSize()});

            /// Build the message to the first hop
            auto routeToFirstBorder = routeToFirstBorderResult.unwrap();
            Message message = Message::build(
                    routeDiscoveryAcknowledgementDatagram,
                    this->deviceID,
                    routeToFirstBorder.route,
                    firstBorder,
                    firstBorderPublicKey.unwrap(),
                    this->deviceKeys);

            return { {MessageTarget::single(firstBorder), message.serialize()} };
        } else {
            // TODO Log out that the route in the routeDiscovery datagram is unknown/invalid
        }

        return {};
    }

    Datagrams Network::rebroadcastRouteDiscovery(Routing::IERP::RouteDiscovery routeDiscovery) {
        /// Get the list of bordercast nodes (destinations to which this should be forwarded) excluding the coveredNodes
        vector<cryptography::UUID> bordercastNodes = this->routingTable.getBordercastNodes(routeDiscovery.coveredNodes);

        /// Add the bordercast nodes to the list of covered nodes
        routeDiscovery.addCoveredNodes(bordercastNodes);

        /// Iterate all forwarding destinations
        Datagram serializedDiscovery = routeDiscovery.serialize();
        Datagrams outgoingDatagrams = {};
        for (auto bordercastNode : bordercastNodes) {

            /// Check if there is a route to the bordercast node
            auto bordercastRouteResult = this->routingTable.getRouteTo(bordercastNode);
            auto bordercastNodePublicKey = this->credentials.getKey(bordercastNode);
            if (bordercastRouteResult.isOk() && bordercastNodePublicKey.isOk()) {

                /// Unwrap the route and build a message traversing it
                auto bordercastRoute = bordercastRouteResult.unwrap();
                Message message = Message::build(
                        serializedDiscovery,
                        this->deviceID,
                        bordercastRoute.route,
                        bordercastNode,
                        bordercastNodePublicKey.unwrap(),
                        this->deviceKeys);

                /// Add the message to the queue
                outgoingDatagrams.emplace_back(MessageTarget::single(bordercastRoute.route[0]), message.serialize());
            } else {
                // TODO Log out that there was no route to the bordercast node
            }
        }

        return outgoingDatagrams;
    }

    Datagrams Network::processRouteDiscovery(const Datagram &datagram) {
        using namespace Routing::IERP;

        /// Deserialize the route discovery datagram
        auto routingResult = RouteDiscovery::fromBuffer(datagram);
        if (routingResult.isErr()) return {};
        RouteDiscovery routeDiscovery = routingResult.unwrap();

        /// Check if this is meant for us
        if (routeDiscovery.destination == this->deviceID) {
            /// Store the public key in the credentials store
            this->credentials.insertKey(routeDiscovery.route.front(), routeDiscovery.origin);
            // TODO Prompt the user when there are conflicting credentials stored

            /// Dispatch an acknowledgement and store the route
            return this->dispatchRouteDiscoveryAcknowledgement(routeDiscovery);
        }

        /// Add ourselves to the route
        routeDiscovery.addHop(this->deviceID);


        /// Check if we know the destination and forward it accordingly
        auto routeToDestinationResult = this->routingTable.getRouteTo(routeDiscovery.destination);
        auto destinationPublicKey = this->credentials.getKey(routeDiscovery.destination);
        if (routeToDestinationResult.isOk() && destinationPublicKey.isOk()) {
            auto routeToDestination = routeToDestinationResult.unwrap();

            Message message = Message::build(
                    routeDiscovery.serialize(),
                    this->deviceID,
                    routeToDestination.route,
                    routeDiscovery.destination,
                    destinationPublicKey.unwrap(),
                    this->deviceKeys);

            return { {MessageTarget::single(routeToDestination.route[0]), message.serialize()} };
        }


        /// Check whether or not the route discovery has exceeded the maximum route length
        if (routeDiscovery.route.size() > MAXIMUM_ROUTE_LENGTH) {
            // TODO Dispatch DeliveryFailureDatagram to sender
            return {};
        }

        /// Rebroadcast the routeDiscovery to all neighbors
        return this->rebroadcastRouteDiscovery(routeDiscovery);
    }

    Datagrams Network::processRouteDiscoveryAcknowledgement(const Datagram &datagram) {
        using namespace scheme::communication::ierp;

        /// Verify the buffer type
        if (!flatbuffers::BufferHasIdentifier(datagram.data(), RouteDiscoveryAcknowledgementDatagramIdentifier()))
            return {}; // INVALID_IDENTIFIER

        /// Verify buffer integrity
        auto verifier = flatbuffers::Verifier(datagram.data(), datagram.size());
        if (!VerifyRouteDiscoveryAcknowledgementDatagramBuffer(verifier))
            return {}; // INVALID_BUFFER

        auto acknowledgement = GetRouteDiscoveryAcknowledgementDatagram(datagram.data());

        /// Deserialize route
        vector<cryptography::UUID> route;
        auto routeBuffer = acknowledgement->route();
        for (uint i = 0; i < routeBuffer->Length(); i++)
            route.emplace_back(routeBuffer->Get(i));

        /// Deserialize public key
        auto pubKeyBuffer = acknowledgement->targetKey();
        auto compressedPubKey = pubKeyBuffer->compressed();
        auto pubKey = cryptography::asymmetric::PublicKey::fromBuffer(compressedPubKey);
        if (pubKey.isErr())
            return {}; // INVALID_PUB_KEY

        /// Insert the route into the routeCache and the public key into the credentialsStore
        this->routeCache.addRoute(route.back(), route);
        this->credentials.insertKey(route.back(), pubKey.unwrap());


        // TODO Possibly dispatch messages in the queue waiting for this
        return {};
    }

    Datagrams Network::processDeliveryFailure(const Datagram &datagram) {
        return {};
    }

    Datagrams Network::processMessageDatagram(const Datagram &datagram) {

        /// Deserialize the route discovery datagram
        auto messageResult = Message::fromBuffer(datagram);
        if (messageResult.isErr()) return {};
        Message message = messageResult.unwrap();

        /// Check whether or not the message is meant for us
        if (message.route.back() == this->deviceID) {
            /// Attempt to retrieve the senders key
            auto keyResult = this->credentials.getKey(message.route.front());

            if (keyResult.isOk()) {
                cryptography::asymmetric::PublicKey key = keyResult.unwrap();

                /// Calculate the shared secret and decrypt the payload
                vector<uint8_t> secret = cryptography::asymmetric::generateSharedSecret(key, this->deviceKeys.priv);
                vector<uint8_t> plaintext = cryptography::symmetric::decrypt(message.payload, secret);

                /// Verify the signature and process the decrypted payload
                if (cryptography::asymmetric::verify(plaintext, message.signature, &key))
                    return this->processDatagram(plaintext);
                // TODO Print a warning when a mismatching signature is received
            } else {
                // TODO Log that the public key to decrypt was unavailable
            }

            return {};
        }

        /// Get our index in the route to determine the next hop
        auto it = find(message.route.begin(), message.route.end(), this->deviceID);
        if (it == message.route.end()) {
            // TODO Log that we received a message we weren't supposed to receive and discard it
            return {};
        }

        /// Get the route to the next hop along the route
        cryptography::UUID nextHop = *(it+1);
        auto routeToNextHopResult = this->routingTable.getRouteTo(nextHop);
        auto nextHopPublicKey = this->credentials.getKey(nextHop);
        if (routeToNextHopResult.isErr() || nextHopPublicKey.isErr()) {
            // TODO Dispatch DeliveryFailureDatagram
            return {};
        }
        auto routeToNextHop = routeToNextHopResult.unwrap();

        /// When the route to the next hop is just one in length forward it as is
        if (routeToNextHop.route.size() == 1)
            return { {MessageTarget::single(nextHop), message.serialize()} };

        /// Otherwise wrap it in another message following routeToNextHop and dispatch that
        Message forwardedMessage = Message::build(
                message.serialize(),
                this->deviceID,
                routeToNextHop.route,
                nextHop,
                nextHopPublicKey.unwrap(),
                this->deviceKeys);

        return { {MessageTarget::single(routeToNextHop.route[0]), message.serialize()} };
    }

    Datagrams Network::processDatagram(const Datagram &datagram) {
        using namespace flatbuffers;

        if (BufferHasIdentifier(datagram.data(), scheme::communication::iarp::AdvertisementDatagramIdentifier()))
            return this->processAdvertisement(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::ierp::RouteDiscoveryDatagramIdentifier()))
            return this->processRouteDiscovery(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::ierp::RouteDiscoveryAcknowledgementDatagramIdentifier()))
            return this->processRouteDiscoveryAcknowledgement(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::DeliveryFailureDatagramIdentifier()))
            return this->processDeliveryFailure(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::MessageDatagramIdentifier()))
            return this->processMessageDatagram(datagram);
        else
            return {}; // TODO Since it is unknown pass it to the parent function as incoming interaction data
    }

}