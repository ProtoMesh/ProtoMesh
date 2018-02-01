#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Brightness.hpp"

namespace ProtoMesh::interaction {

    BrightnessEndpoint::BrightnessEndpoint(const shared_ptr<Network> &network, const cryptography::UUID &target,
                                               uint16_t endpointID) : Endpoint(network, target, endpointID) {}

    void BrightnessEndpoint::getBrightness(RequestType requestType) {
//        this->network->processDatagram({1, 2, 3});
    }

    void BrightnessEndpoint::setBrightness(brightness_t brightness) {
//        this->network->processDatagram({1, 2, 3});
    }

}