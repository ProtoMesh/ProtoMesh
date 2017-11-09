#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

#include "routing.hpp"

namespace ProtoMesh::communication::Routing {

    vector<uint8_t> IARP::Advertisement::serialize() const {

        using namespace scheme::communication::iarp;
        flatbuffers::FlatBufferBuilder builder;

        PUB_HASH_T pubKeyHash = this->pubKey.getHash();
        auto pubKey = this->pubKey.toBuffer(&builder);

        scheme::cryptography::UUID uuid = this->uuid.toScheme();

        vector<scheme::cryptography::UUID> routeEntries;
        for (auto hop : this->route)
            routeEntries.push_back(hop.toScheme());

        auto routeVector = builder.CreateVectorOfStructs(routeEntries);

        auto advertisement = CreateAdvertisementDatagram(builder,
                                                         &uuid,
                                                         pubKey,
                                                         routeVector);

        /// Convert it to a byte array
        builder.Finish(advertisement, AdvertisementDatagramIdentifier());
        uint8_t *buf = builder.GetBufferPointer();
        vector<uint8_t> request_vec(buf, buf + builder.GetSize());

        return request_vec;
    }

    Result<IARP::Advertisement, IARP::Advertisement::AdvertisementDeserializationError>
    IARP::Advertisement::fromBuffer(vector<uint8_t> buffer) {

        using namespace scheme::communication::iarp;

        /// Verify the buffer type
        if (!flatbuffers::BufferHasIdentifier(buffer.data(), AdvertisementDatagramIdentifier()))
            return Err(AdvertisementDeserializationError::INVALID_IDENTIFIER);

        /// Verify buffer integrity
        auto verifier = flatbuffers::Verifier(buffer.data(), buffer.size());
        if (!VerifyAdvertisementDatagramBuffer(verifier))
            return Err(AdvertisementDeserializationError::INVALID_BUFFER);

        auto adv = GetAdvertisementDatagram(buffer.data());

        /// Deserialize public key
        auto pubKeyBuffer = adv->pubKey();
        auto compressedPubKey = pubKeyBuffer->compressed();
        auto pubKey = cryptography::asymmetric::PublicKey::fromBuffer(compressedPubKey);
        if (pubKey.isErr())
            return Err(AdvertisementDeserializationError::INVALID_PUB_KEY);

        /// Deserialize route
        vector<cryptography::UUID> route;
        auto routeBuffer = adv->route();
        for (uint i = 0; i < routeBuffer->Length(); i++)
            route.emplace_back(routeBuffer->Get(i));

        /// Deserialize uuid
        cryptography::UUID uuid(adv->uuid());

        return Ok(Advertisement(uuid, pubKey.unwrap(), route));
    }

    void IARP::Advertisement::addHop(cryptography::UUID uuid) {
        this->route.push_back(uuid);
    }

    Result<vector<cryptography::UUID>, IARP::RouteDiscoveryError> IARP::RoutingTable::getRouteTo(cryptography::UUID uuid) {
        if (routes.find(uuid) != routes.end()) {
            vector<RoutingTableEntry>& availableRoutes = routes.at(uuid);

            uint routeIndex = -1;
            uint routeLength = 9999;
            for (uint i = 0; i < availableRoutes.size(); i++) {
                RoutingTableEntry route = availableRoutes[i];
                // TODO Check for validity of RoutingTableEntry and if it isn't valid anymore delete it.
                if (route.route.size() < routeLength) {
                    routeLength = route.route.size();
                    routeIndex = i;
                }
            }
            return Ok(availableRoutes[routeIndex].route);
        } else
            return Err(RouteDiscoveryError::NO_ROUTE_AVAILABLE);
    }

    void IARP::RoutingTable::processAdvertisement(IARP::Advertisement adv) {
        // TODO Replace this with the real thing
        long currentTime = 0;

        /// Check if we already have a route for this target
        if (routes.find(adv.uuid) != routes.end()) {
            // Possibly add the new route entry
            vector<RoutingTableEntry>& availableRoutes = routes.at(adv.uuid);
            availableRoutes.push_back(RoutingTableEntry(adv, currentTime));
        }

        /// Insert the route
        vector<RoutingTableEntry> availableRoutes({RoutingTableEntry(adv, currentTime)});
        routes.insert({adv.uuid, availableRoutes});
    }

#ifdef UNIT_TESTING

    SCENARIO("Two nodes within the same zone should communicate",
             "[unit_test][module][communication][routing][iarp][advertisement]") {

        GIVEN("An advertisement") {
            cryptography::UUID uuid;
            cryptography::UUID hop1;
            cryptography::UUID hop2;
            cryptography::asymmetric::KeyPair pair(cryptography::asymmetric::generateKeyPair());
            Routing::IARP::Advertisement adv = IARP::Advertisement::build(uuid, pair);

            adv.addHop(hop1);
            adv.addHop(hop2);

            WHEN("it is serialized") {
                vector<uint8_t> serializedAdvertisement = adv.serialize();

                THEN("the serialized advertisement should contain an appropriate identifier") {
                    REQUIRE(flatbuffers::BufferHasIdentifier(serializedAdvertisement.data(),
                                                             scheme::communication::iarp::AdvertisementDatagramIdentifier()));
                }

                AND_WHEN("it is deserialized and reserialized again") {
                    auto result = Routing::IARP::Advertisement::fromBuffer(serializedAdvertisement);
                    Routing::IARP::Advertisement deserializedAdvertisement = result.unwrap();
                    vector<uint8_t> reSerializedAdvertisement = deserializedAdvertisement.serialize();

                    THEN("both advertisements should contain the same data") {
                        REQUIRE(deserializedAdvertisement.route == adv.route);
                        REQUIRE(deserializedAdvertisement.uuid == adv.uuid);
                        REQUIRE(deserializedAdvertisement.pubKey.getHash() == adv.pubKey.getHash());
                    }
                    THEN("both bytestreams should be equal") {
                        REQUIRE(reSerializedAdvertisement == serializedAdvertisement);
                    }
                }
            }

        }

        GIVEN("A routing table and a advertisement") {
            cryptography::UUID uuid;
            cryptography::UUID hop1;
            cryptography::UUID hop2;
            cryptography::UUID hop3;
            cryptography::asymmetric::KeyPair pair(cryptography::asymmetric::generateKeyPair());
            Routing::IARP::Advertisement adv = IARP::Advertisement::build(uuid, pair);
            Routing::IARP::Advertisement adv2 = IARP::Advertisement::build(uuid, pair);
            Routing::IARP::RoutingTable table;

            adv.addHop(hop1);
            adv.addHop(hop2);
            adv.addHop(hop3);

            WHEN("the advertisement is added to the routing table") {
                table.processAdvertisement(adv);
                THEN("a route to the advertiser should be available") {
                    vector<cryptography::UUID> route = table.getRouteTo(uuid).unwrap();
                    vector<cryptography::UUID> expectedRoute({hop3, hop2, hop1});
                    REQUIRE(route == expectedRoute);
                }

                AND_WHEN("a second advertisement with three hops is added to the routing table") {
                    adv2.addHop(hop1);
                    adv2.addHop(hop2);
                    table.processAdvertisement(adv2);

                    THEN("the route to the advertiser should be the shorter of the two") {
                        vector<cryptography::UUID> route = table.getRouteTo(uuid).unwrap();
                        vector<cryptography::UUID> expectedRoute({hop2, hop1});
                        REQUIRE(route == expectedRoute);
                    }
                }
            }
        }
    }

#endif // UNIT_TESTING
}
