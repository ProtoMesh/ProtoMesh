#ifndef PROTOMESH_METADATA_HPP
#define PROTOMESH_METADATA_HPP

#include "../Endpoint.hpp"

#include <string>
#include <vector>

using namespace std;

namespace ProtoMesh::interaction {

    class DeviceMetadata {
        string name;
        vector<EndpointType> endpoints;
    };

    template<>
    class Endpoint<EndpointType::Metadata> : public Endpoint_Base {
    public:
        Endpoint(const shared_ptr<Network> &network);

        EndpointType type() override { return EndpointType::Metadata; }

        void requestMetadata();
    };

}


#endif //PROTOMESH_METADATA_HPP
