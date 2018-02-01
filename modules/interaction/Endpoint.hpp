#ifndef PROTOMESH_ENDPOINT_HPP
#define PROTOMESH_ENDPOINT_HPP

#include <memory>
#include <Network.hpp>
#include <utility>

#include "variable_types.hpp"

#define ENDPOINT_T std::shared_ptr<Endpoint_Base>

namespace ProtoMesh::interaction {

    /// All possible endpoint types
    enum class EndpointType {
        Metadata = 0,
        Temperature,
        Brightness
    };

    /// Endpoint function request type (get/subscription)
    enum class RequestType {
        GET,
        SUBSCRIBE,
        UNSUBSCRIBE
    };

    /// Generic endpoint base class
    class Endpoint_Base {
    protected:
        cryptography::UUID target;
        uint16_t endpointID;
    public:
        shared_ptr<communication::Network> network;

        explicit Endpoint_Base(shared_ptr<Network> network, cryptography::UUID target, uint16_t endpointID)
                : target(target), endpointID(endpointID), network(std::move(network)) {};

        virtual EndpointType type()= 0;
    };

    /// Endpoint | Delegate
    template<EndpointType T>
    class EndpointDelegate {};

    /// Endpoint | Primary template instance
    template<EndpointType T>
    class Endpoint : public Endpoint_Base {
    public:
        shared_ptr<EndpointDelegate<T>> delegate = nullptr;

        Endpoint(const shared_ptr<Network> &network, const cryptography::UUID &target, uint16_t endpointID)
                : Endpoint_Base(network, target, endpointID) {}

        EndpointType type() override { return T; }
    };

    /// Helper to cast generic endpoint pointer to specialized pointer
    template<EndpointType T>
    Endpoint<T>* endpoint_cast(const ENDPOINT_T &endpoint) {
        return dynamic_cast<Endpoint<T> *>(endpoint.get());
    }

}

/// Include all endpoint types
#include "EndpointTypes/Metadata.hpp"

#endif //PROTOMESH_ENDPOINT_HPP
