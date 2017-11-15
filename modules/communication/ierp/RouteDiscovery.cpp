#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "RouteDiscovery.hpp"

namespace ProtoMesh::communication::Routing::IERP {

    void RouteDiscovery::addCoveredNodes(const vector<cryptography::UUID> &nodes) {
        this->coveredNodes.insert(this->coveredNodes.end(), nodes.begin(), nodes.end());
    }

    vector<uint8_t> RouteDiscovery::serialize() const {

        using namespace scheme::communication::ierp;
        flatbuffers::FlatBufferBuilder builder;

        auto originKey = this->origin.toBuffer(&builder);

        scheme::cryptography::UUID destinationID = this->destination.toScheme();

        vector<scheme::cryptography::UUID> coveredNodesList;
        for (auto node : this->coveredNodes) coveredNodesList.push_back(node.toScheme());
        auto coveredNodesVector = builder.CreateVectorOfStructs(coveredNodesList);

        vector<scheme::cryptography::UUID> routeEntries;
        for (auto hop : this->route) routeEntries.push_back(hop.toScheme());
        auto routeVector = builder.CreateVectorOfStructs(routeEntries);

        auto routeDiscovery = CreateRouteDiscoveryDatagram(builder,
                                                           coveredNodesVector,
                                                           originKey,
                                                           &destinationID,
                                                           routeVector,
                                                           sentTimestamp
        );

        /// Convert it to a byte array
        builder.Finish(routeDiscovery, RouteDiscoveryDatagramIdentifier());
        uint8_t *buf = builder.GetBufferPointer();

        return {buf, buf + builder.GetSize()};
    }

    Result<RouteDiscovery, RouteDiscovery::RouteDiscoveryDeserializationError>
    RouteDiscovery::fromBuffer(vector<uint8_t> buffer) {
        using namespace scheme::communication::ierp;

        /// Verify the buffer type
        if (!flatbuffers::BufferHasIdentifier(buffer.data(), RouteDiscoveryDatagramIdentifier()))
            return Err(RouteDiscoveryDeserializationError::INVALID_IDENTIFIER);

        /// Verify buffer integrity
        auto verifier = flatbuffers::Verifier(buffer.data(), buffer.size());
        if (!VerifyRouteDiscoveryDatagramBuffer(verifier))
            return Err(RouteDiscoveryDeserializationError::INVALID_BUFFER);

        auto adv = GetRouteDiscoveryDatagram(buffer.data());

        /// Deserialize the covered nodes list
        vector<cryptography::UUID> coveredNodes;
        auto coveredNodesBuffer = adv->coveredNodes();
        for (uint i = 0; i < coveredNodesBuffer->Length(); i++)
            coveredNodes.emplace_back(coveredNodesBuffer->Get(i));

        /// Deserialize origins public key
        auto originKeyBuffer = adv->origin();
        auto compressedOriginKey = originKeyBuffer->compressed();
        auto originKey = cryptography::asymmetric::PublicKey::fromBuffer(compressedOriginKey);
        if (originKey.isErr())
            return Err(RouteDiscoveryDeserializationError::INVALID_ORIGIN_KEY);

        /// Deserialize destination UUID
        cryptography::UUID destinationID(adv->destination());

        /// Deserialize route
        vector<cryptography::UUID> route;
        auto routeBuffer = adv->route();
        for (uint i = 0; i < routeBuffer->Length(); i++)
            route.emplace_back(routeBuffer->Get(i));

        return Ok(RouteDiscovery(originKey.unwrap(), destinationID, adv->sentTimestamp(), route, coveredNodes));
    }

#ifdef UNIT_TESTING

    SCENARIO("A route between two devices should be discovered",
             "[unit_test][module][communication][routing][ierp]") {

        GIVEN("A route discovery datagram") {
            cryptography::UUID destination;
            cryptography::asymmetric::KeyPair keys(cryptography::asymmetric::generateKeyPair());
            REL_TIME_PROV_T timeProvider(new DummyRelativeTimeProvider(0));
            RouteDiscovery rd = RouteDiscovery::discover(destination, keys.pub, cryptography::UUID(), 0);

            WHEN("it is serialized") {
                vector<uint8_t> serializedRouteDiscovery = rd.serialize();
                THEN("it should have an appropriate identifier") {
                    REQUIRE(flatbuffers::BufferHasIdentifier(serializedRouteDiscovery.data(),
                                                             scheme::communication::ierp::RouteDiscoveryDatagramIdentifier()));
                }

                AND_WHEN("it is deserialized and reserialized again") {
                    RouteDiscovery deserializedRD = RouteDiscovery::fromBuffer(serializedRouteDiscovery).unwrap();
                    vector<uint8_t> reserializedRouteDiscovery = deserializedRD.serialize();

                    THEN("both discoveries should be equal") {
                        REQUIRE(rd.route == deserializedRD.route);
                        REQUIRE(rd.origin == deserializedRD.origin);
                        REQUIRE(rd.destination == deserializedRD.destination);
                        REQUIRE(rd.coveredNodes == deserializedRD.coveredNodes);
                        REQUIRE(rd.sentTimestamp == deserializedRD.sentTimestamp);
                    }
                    THEN("both bytestreams should be equal") {
                        REQUIRE(reserializedRouteDiscovery == serializedRouteDiscovery);
                    }
                }
            }

            WHEN("covered nodes are added") {
                size_t size = rd.coveredNodes.size();
                rd.addCoveredNodes({destination});
                THEN("the list of covered nodes should grow") {
                    REQUIRE(rd.coveredNodes.size() > size);
                }
            }
        }
    }

#endif // UNIT_TESTING

}