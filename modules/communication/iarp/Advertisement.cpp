#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Advertisement.hpp"

namespace ProtoMesh::communication::Routing::IARP {

    vector<uint8_t> Advertisement::serialize() const {

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

    Result<Advertisement, Advertisement::AdvertisementDeserializationError>
    Advertisement::fromBuffer(vector<uint8_t> buffer) {

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

    void Advertisement::addHop(cryptography::UUID uuid) {
        this->route.push_back(uuid);
    }

#ifdef UNIT_TESTING

    SCENARIO("Advertising within the local zone should be possible",
             "[unit_test][module][communication][routing][iarp]") {

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

    }

#endif // UNIT_TESTING
}