#ifndef LUMOS_NETWORK_HPP
#define LUMOS_NETWORK_HPP

#include <memory>
#include <result.h>

#define STORED_REGISTRY_T std::shared_ptr<Registry<vector<uint8_t>>>

#include "../const.hpp"
#include "../Registry/Registry.hpp"

#include "buffers/uuid_generated.h"
#include "buffers/network/node_generated.h"
#include "buffers/network/group_generated.h"

/// Forward declaration of node since it would create a dependency cycle
class Node;
class Group;

struct GroupCreationError {
    enum class Kind { AlreadyPresent };
    Kind kind;
    std::string text;
    GroupCreationError(Kind kind, std::string text) : kind(kind), text(text) {}
};

class Network {
    APIProvider api;

    Crypto::asym::PublicKey masterKey;

    /// Event listeners
    void onNodeRegistryChange(RegistryEntry<vector<uint8_t>> entry);
    void onGroupRegistryChange(RegistryEntry<vector<uint8_t>> entry);

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
    Result<Crypto::UUID, GroupCreationError> createGroup(Crypto::asym::KeyPair authorization, Crypto::UUID);
};

#include "Node/Node.hpp"


#endif //LUMOS_NETWORK_HPP
