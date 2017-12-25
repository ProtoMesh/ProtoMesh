#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "FunctionCallResponse.hpp"

namespace ProtoMesh::interaction::rpc {

    Result<FunctionCallResponse, DeserializationError> FunctionCallResponse::fromBuffer(vector<uint8_t> buffer) {

        using namespace scheme::interaction::rpc;

        /// Verify the buffer type
        if (!flatbuffers::BufferHasIdentifier(buffer.data(), FunctionCallResponseIdentifier()))
            return Err(DeserializationError::INVALID_IDENTIFIER);

        /// Verify buffer integrity
        auto verifier = flatbuffers::Verifier(buffer.data(), buffer.size());
        if (!VerifyFunctionCallResponseBuffer(verifier))
            return Err(DeserializationError::INVALID_BUFFER);

        auto response = GetFunctionCallResponse(buffer.data());

        /// Deserialize parameters
        vector<uint8_t> returnValue(response->returnValue()->begin(), response->returnValue()->end());

        return Ok(FunctionCallResponse(response->transactionID(), response->statusCode(), returnValue));
    }

    vector<uint8_t> FunctionCallResponse::serialize() const {
        using namespace scheme::interaction::rpc;
        flatbuffers::FlatBufferBuilder builder;

        /// Serialize the return value
        auto returnValue = builder.CreateVector(this->returnValue);

        auto functionCall = CreateFunctionCallResponse(builder, this->transactionID, this->statusCode, returnValue);

        /// Convert it to a byte array
        builder.Finish(functionCall, FunctionCallResponseIdentifier());
        uint8_t *buf = builder.GetBufferPointer();

        return {buf, buf + builder.GetSize()};
    }


#ifdef UNIT_TESTING

    SCENARIO("Remote procedure invocation response",
             "[unit_test][module][interaction][rpc]") {

        GIVEN("A FunctionCallResponse") {
            FunctionCallResponse response(250, 20, {0, 1, 2, 3, 4});

            WHEN("it is serialized") {
                vector<uint8_t> serializedResponse = response.serialize();

                THEN("the serialized function response should contain an appropriate identifier") {
                    REQUIRE(flatbuffers::BufferHasIdentifier(serializedResponse.data(),
                                                             scheme::interaction::rpc::FunctionCallResponseIdentifier()));
                }

                AND_WHEN("it is deserialized") {
                    FunctionCallResponse deserializedCall = FunctionCallResponse::fromBuffer(serializedResponse).unwrap();

                    AND_WHEN("it is reserialized again") {
                        vector<uint8_t> reSerializedResponse = deserializedCall.serialize();
                        THEN("both bytestreams should be equal") {
                            REQUIRE(reSerializedResponse == serializedResponse);
                        }
                    }
                }
            }
        }
    }

#endif

}