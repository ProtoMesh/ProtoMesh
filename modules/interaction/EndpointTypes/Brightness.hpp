#ifndef PROTOMESH_BRIGHTNESS_HPP
#define PROTOMESH_BRIGHTNESS_HPP

#include "../Endpoint.hpp"

#include <string>
#include <vector>

using namespace std;

namespace ProtoMesh::interaction {

#define BRIGHTNESS_DELEGATE_T shared_ptr<BrightnessEndpointDelegate>

    class BrightnessEndpointDelegate {
    public:
        virtual void didChangeBrightness(brightness_t brightness) {};
    };

    class BrightnessEndpoint : public Endpoint {
    public:
        /// Constructors
        BrightnessEndpoint(const shared_ptr<Network> &network, const cryptography::UUID &target, uint16_t endpointID);

        /// Superclass functions
        EndpointType type() override { return EndpointType::Brightness; }

        /// Brightness specifics
        void setBrightness(brightness_t brightness);
        void getBrightness(RequestType requestType = RequestType::GET);

        /// Delegates
        BRIGHTNESS_DELEGATE_T delegate;
    };

}


#endif //PROTOMESH_BRIGHTNESS_HPP
