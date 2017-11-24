#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Message.hpp"

namespace ProtoMesh::communication {

    vector<uint8_t> Message::serialize() {

        using namespace scheme::communication;
        flatbuffers::FlatBufferBuilder builder;

        /// Serialize the route
        vector<scheme::cryptography::UUID> routeEntries;
        for (auto hop : this->route)
            routeEntries.push_back(hop.toScheme());

        auto routeVector = builder.CreateVectorOfStructs(routeEntries);

        /// Serialize the payload
        auto payload = builder.CreateVector(this->payload);

        /// Serialize the signature
        // TODO Make this more memory efficient.
        vector<uint8_t> signatureVector(this->signature.begin(), this->signature.end());
        auto signature = builder.CreateVector(signatureVector);

        auto message = CreateMessageDatagram(builder, routeVector, payload, signature);

        /// Convert it to a byte array
        builder.Finish(message, MessageDatagramIdentifier());
        uint8_t *buf = builder.GetBufferPointer();

        return {buf, buf + builder.GetSize()};
    }

    Result<Message, Message::MessageDeserializationError> Message::fromBuffer(vector<uint8_t> buffer) {

        using namespace scheme::communication;

        /// Verify the buffer type
        if (!flatbuffers::BufferHasIdentifier(buffer.data(), MessageDatagramIdentifier()))
            return Err(MessageDeserializationError::INVALID_IDENTIFIER);

        /// Verify buffer integrity
        auto verifier = flatbuffers::Verifier(buffer.data(), buffer.size());
        if (!VerifyMessageDatagramBuffer(verifier))
            return Err(MessageDeserializationError::INVALID_BUFFER);

        auto msg = GetMessageDatagram(buffer.data());

        /// Deserialize route
        vector<cryptography::UUID> route;
        auto routeBuffer = msg->route();
        for (uint i = 0; i < routeBuffer->Length(); i++)
            route.emplace_back(routeBuffer->Get(i));

        /// Deserialize payload
        vector<uint8_t> payload(msg->payload()->begin(), msg->payload()->end());

        /// Deserialize signature
        if (msg->signature()->size() != SIGNATURE_SIZE)
            return Err(MessageDeserializationError::SIGNATURE_SIZE_MISMATCH);
        SIGNATURE_T signature{};
        // TODO Make this more memory efficient.
        copy(msg->signature()->begin(), msg->signature()->end(), signature.begin());

        return Ok(Message(route, payload, signature));
    }

#ifdef UNIT_TESTING

    SCENARIO("Message exchange and verification",
             "[unit_test][module][communication]") {

        GIVEN("A message") {
            cryptography::asymmetric::KeyPair keyPair(cryptography::asymmetric::generateKeyPair());
            cryptography::UUID origin;
            cryptography::UUID hop1;
            cryptography::UUID hop2;
            cryptography::UUID destination;

            vector<cryptography::UUID> route = {hop1, hop2};

            Message msg = Message::build({1, 2, 3}, origin, route, destination, keyPair);

            WHEN("it is serialized") {
                vector<uint8_t> serializedMsg = msg.serialize();

                THEN("the serialized advertisement should contain an appropriate identifier") {
                    REQUIRE(flatbuffers::BufferHasIdentifier(serializedMsg.data(),
                                                             scheme::communication::MessageDatagramIdentifier()));
                }

                AND_WHEN("it is deserialized and reserialized again") {
                    Message deserializedMsg = Message::fromBuffer(serializedMsg).unwrap();
                    vector<uint8_t> reSerializedMsg = deserializedMsg.serialize();

                    THEN("both bytestreams should be equal") {
                        REQUIRE(reSerializedMsg == serializedMsg);
                    }
                }
            }
        }
    }

#endif
}