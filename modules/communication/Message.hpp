#ifndef PROTOMESH_MESSAGE_HPP
#define PROTOMESH_MESSAGE_HPP

#include <utility>
#include <vector>

using namespace std;

#include "uuid.hpp"
#include "asymmetric.hpp"

#include "flatbuffers/flatbuffers.h"
#include "communication/message_generated.h"

namespace ProtoMesh::communication {
    class Message {
        vector<cryptography::UUID> route;
        vector<uint8_t> payload;
        SIGNATURE_T signature;

        enum class MessageDeserializationError {
            INVALID_IDENTIFIER,
            INVALID_BUFFER,
            SIGNATURE_SIZE_MISMATCH
        };

        Message(vector<cryptography::UUID> route, vector<uint8_t> payload, SIGNATURE_T signature)
                : route(std::move(route)), payload(std::move(payload)), signature(signature) {};

    public:
        static Message build(const vector<uint8_t> &payload, cryptography::UUID from, vector<cryptography::UUID> via,
                             cryptography::UUID to, cryptography::asymmetric::KeyPair signer) {
            via.insert(via.begin(), from);
            via.push_back(to);
            SIGNATURE_T signature(cryptography::asymmetric::sign(payload, signer.priv));
            return Message(via, payload, signature);
        }

        vector<uint8_t> serialize();

        static Result<Message, MessageDeserializationError> fromBuffer(vector<uint8_t> buffer);
    };
}


#endif //PROTOMESH_MESSAGE_HPP
