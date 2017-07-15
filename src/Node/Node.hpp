#ifndef LUMOS_NODE_HPP
#define LUMOS_NODE_HPP

#include <vector>
using namespace std;

#include "Network/Network.hpp"
#include "crypto/crypto.hpp"

class Node {
    vector<shared_ptr<Network>> networks;
public:
    Crypto::UUID uuid;
    Crypto::asym::KeyPair pair;

    Node(Crypto::UUID uuid, Crypto::asym::KeyPair keys) : uuid(uuid), pair(keys) {};

    vector<uint8_t> serializeForRegistry();

    void registerAt(shared_ptr<Network> network) {
        this->networks.push_back(network);
    };
};


#endif //LUMOS_NODE_HPP
