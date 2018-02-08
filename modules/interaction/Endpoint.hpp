#ifndef PROTOMESH_ENDPOINT_HPP
#define PROTOMESH_ENDPOINT_HPP

#include <memory>
#include <Network.hpp>
#include <utility>

#include "variable_types.hpp"
#include "rpc/FunctionCall.hpp"
#include "TransmissionHandler.hpp"
#include "Network.hpp"
#include "asymmetric.hpp"

#define ENDPOINT_T std::shared_ptr<Endpoint>

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
    class Endpoint {
    protected:
        cryptography::UUID target;
        uint16_t endpointID;
    public:
        shared_ptr<communication::Network> network;

        explicit Endpoint(shared_ptr<Network> network, cryptography::UUID target, uint16_t endpointID)
                : target(target), endpointID(endpointID), network(std::move(network)) {};

        virtual EndpointType type()= 0;
    };

}

/// Include all endpoint types
#include "EndpointTypes/Brightness.hpp"

#endif //PROTOMESH_ENDPOINT_HPP
