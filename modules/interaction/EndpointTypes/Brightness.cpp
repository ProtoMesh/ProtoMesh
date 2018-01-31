#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Brightness.hpp"

namespace ProtoMesh::interaction {

    Endpoint<EndpointType::Brightness>::Endpoint(const shared_ptr<Network> &network, const cryptography::UUID &target,
                                               uint16_t endpointID) : Endpoint_Base(network, target, endpointID) {
        brightnessDelegate = make_shared<BrightnessDelegate>();
    }

    void Endpoint<EndpointType::Brightness>::getBrightness(RequestType requestType) {
//        this->network->processDatagram({1, 2, 3});
    }

    void Endpoint<EndpointType::Brightness>::setBrightness(brightness_t brightness) {
//        this->network->processDatagram({1, 2, 3});
    }

}