#ifndef PROTOMESH_FUNCTIONCALLRESPONSE_HPP
#define PROTOMESH_FUNCTIONCALLRESPONSE_HPP

#include <utility>

#include "Serializable.hpp"

#include "flatbuffers/flatbuffers.h"
#include "interaction/rpc/FunctionCallResponse_generated.h"

namespace ProtoMesh::interaction::rpc {

    class FunctionCallResponse : public Serializable<FunctionCallResponse> {

        uint8_t transactionID, statusCode;
        vector<uint8_t> returnValue;

#ifdef UNIT_TESTING
    public:
#endif
        FunctionCallResponse(uint8_t transactionID, uint8_t statusCode, vector<uint8_t> returnValue)
                : transactionID(transactionID), statusCode(statusCode), returnValue(std::move(returnValue)) {};

    public:
        /// Serializable overrides
        static Result<FunctionCallResponse, DeserializationError> fromBuffer(vector<uint8_t> buffer);
        vector<uint8_t> serialize() const override;
    };

}


#endif //PROTOMESH_FUNCTIONCALLRESPONSE_HPP
