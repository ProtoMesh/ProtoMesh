#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Metadata.hpp"

namespace ProtoMesh::interaction {

    Endpoint<EndpointType::Metadata>::Endpoint(const shared_ptr<Network> &network, const cryptography::UUID &target,
                                               uint16_t endpointID) : Endpoint_Base(network, target, endpointID) {}

    void Endpoint<EndpointType::Metadata>::getMetadata(RequestType requestType) {
        this->network->processDatagram({1, 2, 3});
    }

}