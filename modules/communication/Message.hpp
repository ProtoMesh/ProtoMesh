#ifndef PROTOMESH_MESSAGE_HPP
#define PROTOMESH_MESSAGE_HPP

#include <utility>
#include <vector>

using namespace std;

#include "uuid.hpp"
#include "symmetric.hpp"
#include "asymmetric.hpp"

#include "flatbuffers/flatbuffers.h"
#include "communication/message_generated.h"

namespace ProtoMesh::communication {
    class Message {
#ifdef UNIT_TESTING
    public:
#endif
        vector<cryptography::UUID> route;
        vector<uint8_t> payload;
        SIGNATURE_T signature;

        enum class MessageDeserializationError {
            INVALID_IDENTIFIER,
            INVALID_BUFFER,
            SIGNATURE_SIZE_MISMATCH
        };

        enum class MessageDecryptionError {
            InvalidSignature
        };

        Message(vector<cryptography::UUID> route, vector<uint8_t> payload, SIGNATURE_T signature)
                : route(std::move(route)), payload(std::move(payload)), signature(signature) {};

    public:
        /// Member functions
        Result<vector<uint8_t>, MessageDecryptionError> decryptPayload(cryptography::asymmetric::PublicKey sender, cryptography::asymmetric::KeyPair recipient);
        vector<uint8_t> serialize();

        /// Constructors
        static Message build(const vector<uint8_t> &payload, vector<cryptography::UUID> route,
                             cryptography::asymmetric::PublicKey destinationKey, cryptography::asymmetric::KeyPair signer);

        static Result<Message, MessageDeserializationError> fromBuffer(vector<uint8_t> buffer);
    };
}


#endif //PROTOMESH_MESSAGE_HPP
