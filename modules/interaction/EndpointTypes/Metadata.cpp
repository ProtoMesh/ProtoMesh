#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Metadata.hpp"

namespace ProtoMesh::interaction {
    Endpoint<EndpointType::Metadata>::Endpoint(const shared_ptr<Network> &network) : Endpoint_Base(network) {}

    void Endpoint<EndpointType::Metadata>::requestMetadata() {
        this->network->processDatagram({1, 2, 3});
    }


}