#ifndef PROTOMESH_BRIGHTNESS_HPP
#define PROTOMESH_BRIGHTNESS_HPP

#include "../Endpoint.hpp"

#include <string>
#include <vector>

using namespace std;

namespace ProtoMesh::interaction {

#define BRIGHTNESS_DELEGATE_T shared_ptr<BrightnessDelegate>

    class BrightnessDelegate {
    public:
        BrightnessDelegate() = default;
        ~BrightnessDelegate() = default;

        virtual void didChangeBrightness(brightness_t &brightness) {};
    };

    template<>
    class Endpoint<EndpointType::Brightness> : public Endpoint_Base {
    public:
        /// Delegates
        BRIGHTNESS_DELEGATE_T brightnessDelegate;

        /// Constructors
        Endpoint(const shared_ptr<Network> &network, const cryptography::UUID &target, uint16_t endpointID);

        /// Superclass functions
        EndpointType type() override { return EndpointType::Brightness; }

        /// Brightness specifics
        void setBrightness(brightness_t brightness);
        void getBrightness(RequestType requestType = RequestType::GET);
    };

}


#endif //PROTOMESH_BRIGHTNESS_HPP
