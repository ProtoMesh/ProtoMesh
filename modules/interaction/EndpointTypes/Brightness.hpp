#ifndef PROTOMESH_BRIGHTNESS_HPP
#define PROTOMESH_BRIGHTNESS_HPP

#include "../Endpoint.hpp"

#include <string>
#include <vector>

using namespace std;

#include "rpc/FunctionCallResponse.hpp"
#include "rpc/FunctionCall.hpp"
#include "flatbuffers/flexbuffers.h"

namespace ProtoMesh::interaction {

#define FN_ID_GET_BRIGHTNESS 0
#define FN_ID_SET_BRIGHTNESS 1
#define BRIGHTNESS_DELEGATE_T shared_ptr<BrightnessEndpointDelegate>

    /// Delegate for software-side endpoint
    class BrightnessEndpointDelegate {
    public:
        virtual void didReceiveBrightness(brightness_t brightness) {};
    };

    /// Endpoint for software-side API
    class BrightnessEndpoint : public Endpoint {
    public:
        /// Constructors
        BrightnessEndpoint(const shared_ptr<Network> &network, const cryptography::UUID &target, uint16_t endpointID);

        /// Superclass functions
        EndpointType type() override { return EndpointType::Brightness; }

        /// Brightness specifics
        void setBrightness(brightness_t brightness);
        void getBrightness(RequestType requestType = RequestType::GET);

        /// Delegate
        BRIGHTNESS_DELEGATE_T delegate = nullptr;

        /// Delegate calls
        void didReceiveBrightness(const Datagram &functionCallResponsePayload);
    };


    /// Handler for state preservation
    // TODO Call subscribed delegates / remotes
    class BrightnessEndpointHandler : public BrightnessEndpoint {
        brightness_t brightness;
    public:
        BrightnessEndpointHandler(const shared_ptr<Network> &network, const cryptography::UUID &target,
                                  uint16_t endpointID);

        Datagram setBrightnessResponse(brightness_t brightness);
        Datagram getBrightnessResponse(RequestType requestType = RequestType::GET);
    };

}


#endif //PROTOMESH_BRIGHTNESS_HPP
