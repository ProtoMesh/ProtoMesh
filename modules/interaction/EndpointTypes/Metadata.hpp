#ifndef PROTOMESH_METADATA_HPP
#define PROTOMESH_METADATA_HPP

#include "../Endpoint.hpp"

namespace ProtoMesh::interaction {

    template<>
    class Endpoint<EndpointType::Metadata> : public Endpoint_Base<EndpointType::Metadata> {
    public:
        void someMetadataFunction();
    };

}


#endif //PROTOMESH_METADATA_HPP
