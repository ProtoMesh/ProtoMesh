#ifdef UNIT_TESTING

#include "catch.hpp"
#include "NetworkSimulator.hpp"

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

        /// Check if this advertisement already traversed us and discard it
        if (std::find(advertisement.route.begin(), advertisement.route.end(), this->deviceID) != advertisement.route.end())
            return {};

        /// Store the route in the routing table and add ourselves to the list
        advertisement.addHop(this->deviceID);
        this->routingTable.processAdvertisement(advertisement);

        /// Store the key in the credentials store
        this->credentials.insertKey(advertisement.uuid, advertisement.pubKey);

        /// Check if it has already travelled to the border of the zone
        /// We have to subtract one since the route already includes us
        if (advertisement.route.size() >= ZONE_RADIUS - 1) return {};

        // TODO Don't rebroadcast to the original sender. Some form of target node white/blacklist maybe
        //      Optionally obviously since not all transmission media might support it.
        return { {MessageTarget::broadcast(), advertisement.serialize()} };
    }

    Datagrams Network::dispatchRouteDiscoveryAcknowledgement(Routing::IERP::RouteDiscovery routeDiscovery) {
        vector<cryptography::UUID> reversedRoute(routeDiscovery.route.rbegin(), routeDiscovery.route.rend());
        reversedRoute.push_back(this->deviceID);
        this->routeCache.addRoute(routeDiscovery.route.front(), reversedRoute);

        using namespace scheme::communication::ierp;
        flatbuffers::FlatBufferBuilder builder;

        /// Serialize the route
        vector<scheme::cryptography::UUID> routeEntries;
        for (auto hop : routeDiscovery.route)
            routeEntries.push_back(hop.toScheme());
        routeEntries.push_back(this->deviceID.toScheme());

        auto routeVector = builder.CreateVectorOfStructs(routeEntries);

        /// Serialize our public key
        auto pubKey = this->deviceKeys.pub.toBuffer(&builder);

        auto routeDiscoveryAcknowledgement = CreateRouteDiscoveryAcknowledgementDatagram(builder, routeVector,
                                                                                         pubKey);

        /// Convert it to a byte array
        builder.Finish(routeDiscoveryAcknowledgement, RouteDiscoveryAcknowledgementDatagramIdentifier());
        uint8_t *buf = builder.GetBufferPointer();
        vector<uint8_t> routeDiscoveryAcknowledgementDatagram({buf, buf + builder.GetSize()});


        auto message = this->sendMessageLocalTo(routeDiscovery.route.back(), routeDiscoveryAcknowledgementDatagram);
        if (message.isOk()) return { message.unwrap() };

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
            auto message = this->sendMessageLocalTo(bordercastNode, serializedDiscovery);
            if (message.isOk()) return { message.unwrap() };
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

        /// Check if this route discovery already traversed us and discard it
        if (std::find(routeDiscovery.route.begin(), routeDiscovery.route.end(), this->deviceID) != routeDiscovery.route.end())
            return {};

        /// Add ourselves to the route
        routeDiscovery.addHop(this->deviceID);


        /// Check if we know the destination and forward it accordingly
        auto message = this->sendMessageLocalTo(routeDiscovery.destination, routeDiscovery.serialize());
        if (message.isOk())
            return { message.unwrap() };


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

        /// Check if we are the final recipient of this route discovery ack and if not forward it accordingly
        /// Since the route in a route discovery acknowledgement is reversed we have to look at route[0]
        auto it = find(route.begin(), route.end(), this->deviceID);
        if (it != route.end() && it != route.begin()) {
            // TODO Possibly use this datagram to derive a partial route and cache that instead of ignoring it
            cryptography::UUID nextBorderNode = *(it-1);

            auto message = this->sendMessageLocalTo(nextBorderNode, datagram);
            if (message.isOk())
                return { message.unwrap() };
        } else if (it == route.end()) {
            // TODO Log that we received a route discovery acknowledgement that wasn't meant for us
        }

        /// Deserialize public key
        auto pubKeyBuffer = acknowledgement->targetKey();
        auto compressedPubKey = pubKeyBuffer->compressed();
        auto pubKey = cryptography::asymmetric::PublicKey::fromBuffer(compressedPubKey);
        if (pubKey.isErr())
            return {}; // INVALID_PUB_KEY

        /// Insert the route into the routeCache and the public key into the credentialsStore
        cryptography::UUID discoveredDevice = route.back();
        this->routeCache.addRoute(discoveredDevice, route);
        this->credentials.insertKey(discoveredDevice, pubKey.unwrap());


        /// Dispatch messages in the routing queue
        if (this->routingQueue.find(discoveredDevice) != this->routingQueue.end()) {
            vector<Datagram> &queuedPayloads = this->routingQueue.at(discoveredDevice);

            for (Datagram payload : queuedPayloads)
                this->queueMessageTo(discoveredDevice, payload);

            this->routingQueue.erase(discoveredDevice);
        }

        return {};
    }

    Datagrams Network::processDeliveryFailure(const Datagram &datagram) {
        return {};
    }

    Datagrams Network::processMessageDatagram(const Datagram &datagram) {

        /// Deserialize the message
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

        /// When the route to the next hop is just one in length forward it as is.
        /// Note that we need to subtract one from the size since we are part of the route.
        if (routeToNextHop.route.size()-1 == 1)
            return { {MessageTarget::single(nextHop), message.serialize()} };

        /// Otherwise wrap it in another message following routeToNextHop and dispatch that
        Message rewrappedMessage = Message::build(
                message.serialize(),
                routeToNextHop.route,
                nextHopPublicKey.unwrap(),
                this->deviceKeys);

        return { {MessageTarget::single(routeToNextHop.route[1]), rewrappedMessage.serialize()} };
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

        this->incomingBuffer.push_back(datagram);
        // TODO Call a callback to process the incomingBuffer
        return {};
    }

    Datagrams Network::discoverDevice(cryptography::UUID device) {
        vector<cryptography::UUID> bordercastNodes = this->routingTable.getBordercastNodes();

        // TODO Add a reasonable timestamp
        Routing::IERP::RouteDiscovery routeDiscovery = Routing::IERP::RouteDiscovery::discover(device, this->deviceKeys.pub, this->deviceID, 0);
        Datagram payload = routeDiscovery.serialize();
        Datagrams outgoingDatagrams;

        for (cryptography::UUID bordercastNode : bordercastNodes) {
            auto datagram = this->sendMessageLocalTo(bordercastNode, payload);
            if (datagram.isOk())
                outgoingDatagrams.push_back(datagram.unwrap());
        }

        return outgoingDatagrams;
    }

    Result<DatagramPacket, Network::MessageSendError> Network::sendMessageLocalTo(cryptography::UUID target,
                                                                                  const Datagram &payload) {
        auto routeResult = this->routingTable.getRouteTo(target);
        auto targetPublicKey = this->credentials.getKey(target);
        if (targetPublicKey.isErr())
            return Err(Network::MessageSendError::TARGET_PUBLIC_KEY_UNKNOWN);
        if (routeResult.isErr())
            return Err(Network::MessageSendError::TARGET_UNREACHABLE);


        auto route = routeResult.unwrap();

        Message message = Message::build(payload, route.route, targetPublicKey.unwrap(), this->deviceKeys);
        DatagramPacket datagram(MessageTarget::single(route.route[1]), message.serialize());

        return Ok(datagram);
    }

    void Network::queueMessageTo(cryptography::UUID target, const Datagram &payload) {
        /// Attempt to deliver the message within the current zone
        auto deliveryResult = this->sendMessageLocalTo(target, payload);

        if (deliveryResult.isOk()) {
            this->outgoingQueue.push_back(deliveryResult.unwrap());
            return;
        }


        /// Attempt to retrieve a route to the destination outside of this zone
        auto routeResult = this->routeCache.getRouteTo(target);
        auto targetKey = this->credentials.getKey(target);

        if (routeResult.isOk() && targetKey.isOk()) {
            auto route = routeResult.unwrap();

            /// Wrap the payload in a message for intrazone transmission
            Message message = Message::build(payload, route.route, targetKey.unwrap(), this->deviceKeys);

            /// Send that message wrapped interzone to the first border node
            auto borderDeliveryResult = this->sendMessageLocalTo(route.route[1], message.serialize());
            if (borderDeliveryResult.isOk()) {
                this->outgoingQueue.push_back(borderDeliveryResult.unwrap());
                return;
            }
        }


        /// Dispatch a route discovery datagram and queue the message
        if (this->routingQueue.find(target) != this->routingQueue.end()) {
            vector<Datagram> &queuedPayloads = this->routingQueue.at(target);
            queuedPayloads.push_back(payload);
            // TODO Maybe don't dispatch another route discovery within a set timeframe to prevent spamming
        } else {
            this->routingQueue.insert({target, {payload}});
        }

        for (DatagramPacket packet : this->discoverDevice(target))
            this->outgoingQueue.push_back(packet);
    }

#ifdef UNIT_TESTING

    SCENARIO("Two devices within the same zone should be able to communicate",
             "[integration_test][module][communication][network][routing][iarp]") {
        GIVEN("five devices (keyPair + id + network) A, w, x, B, y") {
            // Zone layout
            // A <-> w <-> x <-> B <-> y
            NetworkSimulator simulator;
            cryptography::UUID A, w, x, B, y;
            CAPTURE(A);
            CAPTURE(w);
            CAPTURE(x);
            CAPTURE(B);
            CAPTURE(y);

            auto keyA = simulator.createDevice(A, {w});
            auto keyW = simulator.createDevice(w, {A, x});
            auto keyX = simulator.createDevice(x, {w, B});
            auto keyB = simulator.createDevice(B, {x, y});
            auto keyY = simulator.createDevice(y, {B});

            WHEN("A sends an advertisement to its neighbors") {
                REQUIRE(simulator.advertiseNode(A));

                THEN("w, x and B should have a route to A") {
                    vector<cryptography::UUID> expectedRoute_wA = {w, A};
                    vector<cryptography::UUID> expectedRoute_xA = {x, w, A};
                    vector<cryptography::UUID> expectedRoute_BA = {B, x, w, A};

                    REQUIRE(simulator.getNode(w).unwrap()->network.routingTable.getRouteTo(A).unwrap().route == expectedRoute_wA);
                    REQUIRE(simulator.getNode(x).unwrap()->network.routingTable.getRouteTo(A).unwrap().route == expectedRoute_xA);
                    REQUIRE(simulator.getNode(B).unwrap()->network.routingTable.getRouteTo(A).unwrap().route == expectedRoute_BA);
                }

                THEN("y should not have a route to A") {
                    REQUIRE(simulator.getNode(y).unwrap()->network.routingTable.getRouteTo(A).isErr());
                }

                THEN("B should have the public key of A") {
                    REQUIRE(simulator.getNode(B).unwrap()->network.credentials.getKey(A).isOk());
                }
            }
        }
    }

    SCENARIO("Two devices in different zones should be able to communicate",
             "[integration_test][module][communication][network][routing][ierp]") {
        GIVEN("seven devices (keyPair + id + network) A, w, x, B, y, z, C") {
            // Zone layout
            // A <-> w <-> x <-> B <-> y <-> z <-> C
            NetworkSimulator simulator;
            cryptography::UUID A = cryptography::UUID::fromNumber(0),
                    w = cryptography::UUID::fromNumber(0x37), // 55
                    x = cryptography::UUID::fromNumber(0x42), // 66
                    B = cryptography::UUID::fromNumber(1),
                    y = cryptography::UUID::fromNumber(0x4d), // 77 -> TODO Message gets stuck right here. Probably not re-wrapped at B
                    z = cryptography::UUID::fromNumber(0x58), // 88
                    C = cryptography::UUID::fromNumber(2);
            vector<cryptography::UUID> nodes = {A, w, x, B, y, z, C};
            CAPTURE(A);
            CAPTURE(w);
            CAPTURE(x);
            CAPTURE(B);
            CAPTURE(y);
            CAPTURE(z);
            CAPTURE(C);

            auto keyA = simulator.createDevice(A, {w});
            auto keyW = simulator.createDevice(w, {A, x});
            auto keyX = simulator.createDevice(x, {w, B});
            auto keyB = simulator.createDevice(B, {x, y});
            auto keyY = simulator.createDevice(y, {B, z});
            auto keyZ = simulator.createDevice(z, {y, C});
            auto keyC = simulator.createDevice(C, {z});

            WHEN("All devices advertise themselves") {
                NetworkSimulationNode* nodeA = simulator.getNode(A).unwrap();
                NetworkSimulationNode* nodeC = simulator.getNode(C).unwrap();

                for (auto node : nodes)
                    REQUIRE(simulator.advertiseNode(node));

                THEN("C should have B as its bordercast node") {
                    vector<cryptography::UUID> expectedBordercastNodes = {B};
                    REQUIRE(nodeC->network.routingTable.getBordercastNodes() == expectedBordercastNodes);

                    AND_WHEN("C sends a route discovery in search of A") {
                        Datagrams routeDiscoveryDatagrams = nodeC->network.discoverDevice(A);
                        simulator.processDatagrams(routeDiscoveryDatagrams, C);

                        THEN("C should have cached a route of A") {
                            REQUIRE(nodeC->network.routeCache.getRouteTo(A).isOk());
                        }
                    }
                }

                WHEN("A is instructed to send a message to C") {
                    Datagram payload = {1, 2, 3, 4, 5};
                    nodeA->network.queueMessageTo(C, payload);

                    THEN("A should have a message datagram in its outgoingDatagrams buffer") {
                        REQUIRE(nodeA->network.outgoingQueue.size() == 1);
                        REQUIRE(nodeA->network.routingQueue.size() == 1);
                        DatagramPacket discovery = nodeA->network.outgoingQueue.back();
                        Datagram discoveryData = get<1>(discovery);

                        CAPTURE(discoveryData);

                        bool isMessageDatagram = flatbuffers::BufferHasIdentifier(
                                discoveryData.data(), scheme::communication::MessageDatagramIdentifier());
                        REQUIRE(isMessageDatagram);

                        WHEN("the route discovery is processed by the network") {
                            simulator.processMessageQueueOf(A);

                            THEN("A should wrap the payload in a message and move it into the outgoingQueue") {
                                REQUIRE(nodeA->network.routingQueue.empty());
                                REQUIRE(nodeA->network.outgoingQueue.size() == 1);

                                WHEN("the original message is now being dispatched") {
                                    simulator.processMessageQueueOf(A);

                                    THEN("the incoming buffer of C should contain the message sent by A") {
                                        REQUIRE(nodeC->network.incomingBuffer.size() == 1);
                                        REQUIRE(nodeC->network.incomingBuffer.back() == payload);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

#endif // UNIT_TESTING
}