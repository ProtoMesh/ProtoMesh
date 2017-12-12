#include "NetworkSimulator.hpp"

namespace ProtoMesh {

    cryptography::asymmetric::KeyPair
    NetworkSimulator::createDevice(cryptography::UUID deviceID, vector<cryptography::UUID> neighbors) {
        cryptography::asymmetric::KeyPair key = cryptography::asymmetric::generateKeyPair();
        Network net(deviceID, key, this->timeProvider);

        this->nodes.insert({deviceID, NetworkSimulationNode(net, move(neighbors))});

        return key;
    }

    Result<NetworkSimulationNode *, NetworkSimulator::NetworkNodeError> NetworkSimulator::getNode(cryptography::UUID node) {
        if (nodes.find(node) != nodes.end()) {
            return Ok( &(nodes.at(node)) );
        } else
            return Err(NetworkNodeError::NODE_NOT_FOUND);
    }

    bool NetworkSimulator::hasNeighbor(cryptography::UUID node, cryptography::UUID neighbor) {
        auto nodeResult = this->getNode(node);
        if (nodeResult.isErr()) return false;

        vector<cryptography::UUID> neighbors = nodeResult.unwrap()->neighbors;

        return find(neighbors.begin(), neighbors.end(), neighbor) != neighbors.end();
    }

    bool NetworkSimulator::advertiseNode(cryptography::UUID nodeID) {
        auto nodeResult = this->getNode(nodeID);
        if (nodeResult.isErr()) return false;

        NetworkSimulationNode* node = nodeResult.unwrap();

        auto advertisement = Routing::IARP::Advertisement::build(node->network.deviceID, node->network.deviceKeys);

        for (cryptography::UUID neighbor : node->neighbors)
            this->sendMessageTo(neighbor, advertisement.serialize());

        return true;
    }

    void NetworkSimulator::sendMessageTo(cryptography::UUID target, vector<uint8_t> message) {
        auto nodeResult = this->getNode(target);
        if (nodeResult.isErr())
            return; // Node is not found so just exit. TODO Print a warning
        auto node = nodeResult.unwrap();

        this->processDatagrams(node->network.processDatagram(message), target);
    }

    void NetworkSimulator::processDatagrams(Datagrams datagrams, cryptography::UUID senderID) {
        auto senderResult = this->getNode(senderID);
        if (senderResult.isErr()) return;
        auto sender = senderResult.unwrap();

        MessageTarget msgTarget(MessageTarget::Type::SINGLE);
        Datagram datagram;
        for (auto& p : datagrams) {
            tie(msgTarget, datagram) = move(p);

            switch (msgTarget.type) {
                case MessageTarget::Type::SINGLE:
                    if (!this->hasNeighbor(senderID, msgTarget.target)) {
                        cout << "ERROR: Attempted to deliver message to a node which wasn't its neighbor!" << endl;
                        cout << "ERROR: Sender: " << senderID << endl;
                        cout << "ERROR: Recipient: " << msgTarget.target << endl;
                    } else {
                        this->sendMessageTo(msgTarget.target, datagram);
                    }
                    break;
                case MessageTarget::Type::BROADCAST:
                    for (cryptography::UUID neighbor : sender->neighbors)
                        this->sendMessageTo(neighbor, datagram);
                    break;
            }
        }
    }

    void NetworkSimulator::processMessageQueueOf(cryptography::UUID nodeID) {
        auto nodeResult = this->getNode(nodeID);
        if (nodeResult.isErr()) return;
        auto node = nodeResult.unwrap();

        // TODO Replace this with a network.take() function or smth similar
        Datagrams datagrams(node->network.outgoingQueue.begin(), node->network.outgoingQueue.end());
        node->network.outgoingQueue.clear();

        this->processDatagrams(datagrams, nodeID);
    }
}