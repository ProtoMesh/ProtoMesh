#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "FunctionCall.hpp"

namespace ProtoMesh::interaction::rpc {

    vector<uint8_t> FunctionCall::serialize() const {
        using namespace scheme::interaction::rpc;
        flatbuffers::FlatBufferBuilder builder;

        /// Serialize the signature
        // TODO Make this more memory efficient.
        vector<uint8_t> signatureVector(this->signature.begin(), this->signature.end());
        auto signature = builder.CreateVector(signatureVector);

        /// Serialize the parameters
        auto parameter = builder.CreateVector(this->parameter);

        auto functionCall = CreateFunctionCall(builder, this->endpointID, this->function, this->transactionID,
                                               parameter, signature);

        /// Convert it to a byte array
        builder.Finish(functionCall, FunctionCallIdentifier());
        uint8_t *buf = builder.GetBufferPointer();

        return {buf, buf + builder.GetSize()};
    }

    Result<FunctionCall, DeserializationError> FunctionCall::fromBuffer(vector<uint8_t> buffer) {

        using namespace scheme::interaction::rpc;

        /// Verify the buffer type
        if (!flatbuffers::BufferHasIdentifier(buffer.data(), FunctionCallIdentifier()))
            return Err(DeserializationError::INVALID_IDENTIFIER);

        /// Verify buffer integrity
        auto verifier = flatbuffers::Verifier(buffer.data(), buffer.size());
        if (!VerifyFunctionCallBuffer(verifier))
            return Err(DeserializationError::INVALID_BUFFER);

        auto call = GetFunctionCall(buffer.data());

        /// Deserialize parameters
        vector<uint8_t> parameter(call->parameter()->begin(), call->parameter()->end());

        /// Deserialize the signature
        if (call->signature()->size() != SIGNATURE_SIZE)
            return Err(DeserializationError::SIGNATURE_SIZE_MISMATCH);
        SIGNATURE_T signature{};
        // TODO Make this more memory efficient.
        copy(call->signature()->begin(), call->signature()->end(), signature.begin());

        return Ok(FunctionCall(call->endpointID(), call->function(), call->transactionID(), parameter, signature));
    }


#ifdef UNIT_TESTING

    SCENARIO("Remote procedure invocation",
             "[unit_test][module][interaction][rpc]") {

        GIVEN("A FunctionCall") {

            cryptography::asymmetric::KeyPair pair = cryptography::asymmetric::generateKeyPair();
            FunctionCall call = FunctionCall::create(65011, 250, 20, {0, 1, 2, 3, 4}, pair);

            WHEN("it is serialized") {
                vector<uint8_t> serializedCall = call.serialize();

                THEN("the serialized function call should contain an appropriate identifier") {
                    REQUIRE(flatbuffers::BufferHasIdentifier(serializedCall.data(),
                                                             scheme::interaction::rpc::FunctionCallIdentifier()));
                }

                AND_WHEN("it is deserialized") {
                    FunctionCall deserializedCall = FunctionCall::fromBuffer(serializedCall).unwrap();

                    AND_WHEN("it is reserialized again") {
                        vector<uint8_t> reSerializedCall = deserializedCall.serialize();
                        THEN("both bytestreams should be equal") {
                            REQUIRE(reSerializedCall == serializedCall);
                        }
                    }
                }

                // TODO Write a test and function to check the signature
            }
        }
    }

#endif
}