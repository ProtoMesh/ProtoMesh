#ifndef PROTOMESH_METADATA_HPP
#define PROTOMESH_METADATA_HPP

#include "../Endpoint.hpp"

namespace ProtoMesh::interaction {

    template<>
    class Endpoint<EndpointType::Metadata> : public Endpoint_Base {
    public:
        EndpointType type() override { return EndpointType::Metadata; }

        void someMetadataFunction();
    };

}


#endif //PROTOMESH_METADATA_HPP
