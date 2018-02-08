#ifndef PROTOMESH_FUNCTIONCALLRESPONSE_HPP
#define PROTOMESH_FUNCTIONCALLRESPONSE_HPP

#include <utility>

#include "Serializable.hpp"

#include "flatbuffers/flatbuffers.h"
#include "interaction/rpc/FunctionCallResponse_generated.h"

namespace ProtoMesh::interaction::rpc {

    class FunctionCallResponse : public Serializable<FunctionCallResponse> {
        uint16_t endpointID;
        uint8_t function;
        uint8_t statusCode;
        vector<uint8_t> returnValue;

    public:
        FunctionCallResponse(uint16_t endpointID, uint8_t function, uint8_t statusCode, vector<uint8_t> returnValue)
                : endpointID(endpointID), function(function), statusCode(statusCode), returnValue(std::move(returnValue)) {};

        /// Serializable overrides
        static Result<FunctionCallResponse, DeserializationError> fromBuffer(vector<uint8_t> buffer);
        vector<uint8_t> serialize() const override;
    };

}


#endif //PROTOMESH_FUNCTIONCALLRESPONSE_HPP
