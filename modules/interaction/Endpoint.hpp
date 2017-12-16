#ifndef PROTOMESH_ENDPOINT_HPP
#define PROTOMESH_ENDPOINT_HPP

namespace ProtoMesh::interaction {

    /// All possible endpoint types
    enum class EndpointType {
        Metadata = 0,
        Color,
        Temperature,
        Brightness,
        Authorization
    };


    /// Generic endpoint base class
    template<EndpointType T>
    class Endpoint_Base {
    public:
        EndpointType type();
    };


    /// Primary template instance
    template<EndpointType T>
    class Endpoint : public Endpoint_Base<T> {
    };

}

/// Include all endpoint types
#include "EndpointTypes/Metadata.hpp"

#endif //PROTOMESH_ENDPOINT_HPP
