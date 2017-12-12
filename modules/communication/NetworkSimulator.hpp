#ifndef PROTOMESH_NETWORKSIMULATOR_HPP
#define PROTOMESH_NETWORKSIMULATOR_HPP
#ifdef UNIT_TESTING

#include <unordered_map>

#include <iostream>

using namespace std;

#include "result.h"
#include "Network.hpp"
#include "uuid.hpp"
#include "asymmetric.hpp"
#include "RelativeTimeProvider.hpp"

namespace ProtoMesh::communication {
    class NetworkSimulationNode {
    public:
        Network network;
        vector<cryptography::UUID> neighbors;

        NetworkSimulationNode(Network net, vector<cryptography::UUID> neighbors) : network(move(net)), neighbors(move(neighbors)) {}
    };

    class NetworkSimulator {
        unordered_map<cryptography::UUID, NetworkSimulationNode> nodes;
        REL_TIME_PROV_T timeProvider;

        void sendMessageTo(cryptography::UUID target, vector<uint8_t> message);
    public:
        enum class NetworkNodeError {
            NODE_NOT_FOUND
        };


        NetworkSimulator() : timeProvider(new DummyRelativeTimeProvider(0)) {}


        cryptography::asymmetric::KeyPair createDevice(cryptography::UUID deviceID, vector<cryptography::UUID> neighbors);

        Result<NetworkSimulationNode*, NetworkNodeError> getNode(cryptography::UUID node);
        bool hasNeighbor(cryptography::UUID node, cryptography::UUID neighbor);

        bool advertiseNode(cryptography::UUID nodeID);
        void processDatagrams(Datagrams datagrams, cryptography::UUID sender);
        void processMessageQueueOf(cryptography::UUID nodeID);
    };

}

#endif //UNIT_TESTING
#endif //PROTOMESH_NETWORKSIMULATOR_HPP
