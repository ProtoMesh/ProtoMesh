#ifndef LUMOS_GROUP_HPP
#define LUMOS_GROUP_HPP

#include <memory>
#include <Network/Network.hpp>

class Group {
    shared_ptr<Network> parentNetwork;
    Crypto::UUID uuid;
    Crypto::asym::PublicKey publicKey;

    bool loadGroup(Crypto::UUID uuid) {
        return false;
    }
public:
    Group(shared_ptr<Network> network, Crypto::UUID uuid, Crypto::asym::PublicKey key) : parentNetwork(network), uuid(uuid), publicKey(key) {}
    /// Create the group
    static Group createGroup(shared_ptr<Network> network){
        Crypto::UUID uuid;
        Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());

        // TODO Add private key
        return Group(network, uuid, pair.pub);
    }
    /// Load the group
//    Group(shared_ptr<Network> network, Crypto::UUID uuid) : parentNetwork(network), uuid(uuid) {
//        if (!loadGroup(uuid)) createGroup(uuid);
//    }
};


#endif //LUMOS_GROUP_HPP
