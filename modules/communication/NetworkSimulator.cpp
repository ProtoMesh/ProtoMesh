#include "NetworkSimulator.hpp"

namespace ProtoMesh {

    cryptography::asymmetric::KeyPair
    NetworkSimulator::createDevice(cryptography::UUID deviceID, vector<cryptography::UUID> neighbors) {
        cryptography::asymmetric::KeyPair key = cryptography::asymmetric::generateKeyPair();
        Network net(deviceID, key, this->timeProvider);

        this->nodes.insert({deviceID, NetworkNode(net, move(neighbors))});

        return key;
    }

    Result<NetworkNode *, NetworkSimulator::NetworkNodeError> NetworkSimulator::getNode(cryptography::UUID node) {
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
        if (nodeResult.isErr())
            return false;


        NetworkNode* node = nodeResult.unwrap();

        auto advertisement = Routing::IARP::Advertisement::build(node->network.deviceID, node->network.deviceKeys);

//        cout << "Advertising node " << nodeID << " to nodes: " << endl;
        for (cryptography::UUID neighbor : node->neighbors) {
//            cout << "\t" << neighbor << endl;
            this->sendMessageTo(neighbor, advertisement.serialize(), nodeID);
        }

        return true;
    }

    void NetworkSimulator::sendMessageTo(cryptography::UUID target, vector<uint8_t> message, cryptography::UUID from) {
//        cout << "Sending message to " << target << endl;

        auto nodeResult = this->getNode(target);
        if (nodeResult.isErr())
            return; // Node is not found so just exit. TODO Print a warning
        auto node = nodeResult.unwrap();

        Datagrams datagrams = node->network.processDatagram(message);

        MessageTarget msgTarget(MessageTarget::Type::SINGLE);
        Datagram datagram;
        for (auto& p : datagrams) {
            tie(msgTarget, datagram) = move(p);

            switch (msgTarget.type) {
                case MessageTarget::Type::SINGLE:
                    if (!this->hasNeighbor(from, target)) {
                        cout << "ERROR: Attempted to deliver message to a node which wasn't its neighbor!" << endl;
                        cout << "ERROR: Sender: " << from << endl;
                        cout << "ERROR: Recipient: " << target << endl;
                    } else {
//                        cout << "\tResult sent to: " << msgTarget.target << endl;
                        this->sendMessageTo(msgTarget.target, datagram, target);
                    }
                    break;
                case MessageTarget::Type::BROADCAST:
//                    cout << "\tResult broadcasted to:" << endl;
                    for (cryptography::UUID neighbor : node->neighbors)
                        this->sendMessageTo(neighbor, datagram, target);
                    break;
            }
        }
    }
}