#ifndef PROTOMESH_FUNCTIONCALL_HPP
#define PROTOMESH_FUNCTIONCALL_HPP

#include <utility>
#include <vector>

using namespace std;

#include "Serializable.hpp"
#include "asymmetric.hpp"

#include "flatbuffers/flatbuffers.h"
#include "interaction/rpc/FunctionCall_generated.h"


namespace ProtoMesh::interaction::rpc {

    class FunctionCall : public Serializable<FunctionCall> {
        uint16_t endpointID;
        uint8_t function, transactionID;
        vector<uint8_t> parameter;

        SIGNATURE_T signature;

        FunctionCall(uint16_t endpointID, uint8_t function, uint8_t transactionID, vector<uint8_t> parameter,
                     SIGNATURE_T signature) : endpointID(endpointID), function(function), transactionID(transactionID),
                                              parameter(std::move(parameter)), signature(signature) {};

    public:
        static FunctionCall create(uint16_t endpointID, uint8_t function, uint8_t transactionID, vector<uint8_t> parameter, cryptography::asymmetric::KeyPair signer) {
            vector<uint8_t> signatureText = {
                    (uint8_t) (endpointID & 0xFF),
                    (uint8_t) ((endpointID >> 8) & 0xFF),
                    function,
                    transactionID
            };
            signatureText.insert(signatureText.end(), parameter.begin(), parameter.end());

            /// Sign the payload
            SIGNATURE_T signature(cryptography::asymmetric::sign(signatureText, signer.priv));

            return FunctionCall(endpointID, function, transactionID, parameter, signature);
        }

        /// Serializable overrides
        static Result<FunctionCall, DeserializationError> fromBuffer(vector<uint8_t> buffer);
        vector<uint8_t> serialize() const override;
    };

}



#endif //PROTOMESH_FUNCTIONCALL_HPP
