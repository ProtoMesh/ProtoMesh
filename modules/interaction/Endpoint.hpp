#ifndef PROTOMESH_ENDPOINT_HPP
#define PROTOMESH_ENDPOINT_HPP

#include <memory>
#include <Network.hpp>
#include <utility>

#define ENDPOINT_T std::shared_ptr<Endpoint_Base>

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
    class Endpoint_Base {
    public:
        shared_ptr<communication::Network> network;

        explicit Endpoint_Base(shared_ptr<Network> network) : network(std::move(network)) {};

        virtual EndpointType type()= 0;
    };


    /// Primary template instance
    template<EndpointType T>
    class Endpoint : public Endpoint_Base {
    public:
        explicit Endpoint(const shared_ptr<Network> &network) : Endpoint_Base(network) {};

        EndpointType type() override { return T; }
    };

}

/// Include all endpoint types
#include "EndpointTypes/Metadata.hpp"

#endif //PROTOMESH_ENDPOINT_HPP
