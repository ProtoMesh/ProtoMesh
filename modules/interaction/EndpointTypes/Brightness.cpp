#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Brightness.hpp"

namespace ProtoMesh::interaction {

    BrightnessEndpoint::BrightnessEndpoint(const shared_ptr<Network> &network, const cryptography::UUID &target,
                                               uint16_t endpointID) : Endpoint(network, target, endpointID) {}

    void BrightnessEndpoint::getBrightness(RequestType requestType) {
        flexbuffers::Builder parameters;
        parameters.Add(static_cast<unsigned int>(requestType));
        parameters.Finish();

        rpc::FunctionCall call = rpc::FunctionCall::create(this->endpointID, FN_ID_GET_BRIGHTNESS, parameters.GetBuffer(), this->network->getKeys());

        // TODO Write test
        this->network->queueMessageTo(this->target, call.serialize());
    }

    void BrightnessEndpoint::setBrightness(brightness_t brightness) {
        // TODO Dispatch a function call to set the brightness
    }

    void BrightnessEndpoint::didReceiveBrightness(const Datagram &functionCallResponsePayload) {
        auto root = flexbuffers::GetRoot(functionCallResponsePayload);
        if (root.IsFloat() && this->delegate != nullptr)
            this->delegate->didReceiveBrightness(root.AsFloat());
    }

    BrightnessEndpointHandler::BrightnessEndpointHandler(const shared_ptr<Network> &network,
                                                         const cryptography::UUID &target, uint16_t endpointID)
            : BrightnessEndpoint(network, target, endpointID) {}

    Datagram BrightnessEndpointHandler::setBrightnessResponse(brightness_t brightness) {
        this->brightness = brightness;

        uint8_t statusCode = 0;
        rpc::FunctionCallResponse response(this->endpointID, FN_ID_SET_BRIGHTNESS, statusCode, {});
        return response.serialize();
    }

    Datagram BrightnessEndpointHandler::getBrightnessResponse(RequestType requestType) {
        flexbuffers::Builder fbb;
        fbb.Float(this->brightness);
        fbb.Finish();

        uint8_t statusCode = 0;
        rpc::FunctionCallResponse response(this->endpointID, FN_ID_GET_BRIGHTNESS, statusCode, fbb.GetBuffer());
        return response.serialize();
    }
}