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

        /// Store the route in the routing table and add ourselves to the list
        this->routingTable.processAdvertisement(advertisement);
        advertisement.addHop(this->deviceID);

        /// Check if it has already travelled to the border of the zone
        if (advertisement.route.size() > ZONE_RADIUS) return {};

        // TODO Don't rebroadcast to the original sender. Some form of target node white/blacklist maybe
        //      Optionally obviously since not all transmission media might support it.
        return { {MessageTarget::broadcast(), advertisement.serialize()} };
    }

    Datagrams Network::processRouteDiscovery(const Datagram &datagram) {
        using namespace Routing::IERP;

        /// Deserialize the route discovery datagram
        auto routingResult = RouteDiscovery::fromBuffer(datagram);
        if (routingResult.isErr()) return {};
        RouteDiscovery routeDiscovery = routingResult.unwrap();

        /// Check if this is meant for us
        if (routeDiscovery.destination == this->deviceID) {
            cryptography::UUID firstBorder = routeDiscovery.route.back();
            auto routeToFirstBorderResult = this->routingTable.getRouteTo(firstBorder);

            if (routeToFirstBorderResult.isOk()) {
                // TODO Store the route for later use (requires an IERP routing table equivalent)

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
                        routeToFirstBorder.publicKey,
                        this->deviceKeys);

                return { {MessageTarget::single(firstBorder), message.serialize()} };
            } else {
                // TODO Log out that the route in the routeDiscovery datagram is unknown/invalid
            }
        }

        /// Add ourselves to the route
        routeDiscovery.addHop(this->deviceID);


        /// Check if we know the destination and forward it accordingly
        auto routeToDestinationResult = this->routingTable.getRouteTo(routeDiscovery.destination);
        if (routeToDestinationResult.isOk()) {
            auto routeToDestination = routeToDestinationResult.unwrap();

            Message message = Message::build(
                    routeDiscovery.serialize(),
                    this->deviceID,
                    routeToDestination.route,
                    routeDiscovery.destination,
                    routeToDestination.publicKey,
                    this->deviceKeys);

            return { {MessageTarget::single(routeToDestination.route[0]), message.serialize()} };
        }


        /// Check whether or not the route discovery has exceeded the maximum route length
        if (routeDiscovery.route.size() > MAXIMUM_ROUTE_LENGTH) {
            // TODO Dispatch DeliveryFailureDatagram to sender
            return {};
        }


        /// Get the list of bordercast nodes (destinations to which this should be forwarded) excluding the coveredNodes
        vector<cryptography::UUID> bordercastNodes = this->routingTable.getBordercastNodes(routeDiscovery.coveredNodes);
        routeDiscovery.addCoveredNodes(bordercastNodes);

        /// Iterate all forwarding destinations
        Datagram serializedDiscovery = routeDiscovery.serialize();
        Datagrams outgoingDatagrams = {};
        for (auto bordercastNode : bordercastNodes) {

            /// Check if there is a route to the bordercast node
            auto bordercastRouteResult = this->routingTable.getRouteTo(bordercastNode);
            if (bordercastRouteResult.isOk()) {

                /// Unwrap the route and build a message for it
                auto bordercastRoute = bordercastRouteResult.unwrap();
                Message message = Message::build(
                        serializedDiscovery,
                        this->deviceID,
                        bordercastRoute.route,
                        bordercastNode,
                        bordercastRoute.publicKey,
                        this->deviceKeys);

                /// Add the message to the queue
                outgoingDatagrams.emplace_back(MessageTarget::single(bordercastRoute.route[0]), message.serialize());
            } else {
                // TODO Log out that there was no route to the bordercast node
            }
        }

        return outgoingDatagrams;
    }

    Datagrams Network::processDeliveryFailure(const Datagram &datagram) {
        return {};
    }

    Datagrams Network::processMessageDatagram(const Datagram &datagram) {



        return {};
    }

    Datagrams Network::processDatagram(const Datagram &datagram) {
        using namespace flatbuffers;

        if (BufferHasIdentifier(datagram.data(), scheme::communication::iarp::AdvertisementDatagramIdentifier()))
            return processAdvertisement(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::ierp::RouteDiscoveryDatagramIdentifier()))
            return processRouteDiscovery(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::DeliveryFailureDatagramIdentifier()))
            return processDeliveryFailure(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::MessageDatagramIdentifier()))
            return processMessageDatagram(datagram);
        else
            return {};
    }

}