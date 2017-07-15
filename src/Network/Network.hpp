#ifndef LUMOS_NETWORK_HPP
#define LUMOS_NETWORK_HPP

#include <memory>

#define STORED_REGISTRY_T std::shared_ptr<Registry<vector<uint8_t>>>

#include "../const.hpp"
#include "../Registry/Registry.hpp"

#include "buffers/network/node_generated.h"
#include "buffers/uuid_generated.h"

/// Forward declaration of node since it would create a dependency cycle
class Node;
class Group;

class Network {
    APIProvider api;

    Crypto::asym::PublicKey masterKey;

public:
    /// Constructor
    Network(APIProvider api, Crypto::asym::PublicKey masterKey);

    /// Registries
    map<string, STORED_REGISTRY_T> registries;
    BCAST_SOCKET_T registryBcast;
    STORED_REGISTRY_T loadRegistry(string name);
    STORED_REGISTRY_T getRegistry(string name);

    /// Networking
    void tick(unsigned int timeoutMS);

    /// Nodes
    void registerNode(Crypto::UUID uid, vector<uint8_t> node, Crypto::asym::KeyPair authorization);

    /// Groups
//    shared_ptr<Group> create_group();
//    shared_ptr<Group> get_group(Crypto::UUID uid);
};

#include "Node/Node.hpp"
#include "Group/Group.hpp"


#endif //LUMOS_NETWORK_HPP
