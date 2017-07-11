#ifndef LUMOS_NETWORK_H
#define LUMOS_NETWORK_H

#include "../api/time.hpp"
#include "../api/network.hpp"
#include "../api/storage.hpp"
#include "../Registry/Registry.hpp"
#include "../crypto/crypto.hpp"
#include "Network.hpp"

class NetworkManager {
    NetworkProvider* net;
    StorageProvider* stor;
    REL_TIME_PROV_T time;
public:
    NetworkManager(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider) : net(net), stor(stor), time(relTimeProvider) {};

    Crypto::asym::KeyPair createNetwork() {
        // TODO Initialize the data in the registries for that network
        return Crypto::asym::generateKeyPair();
    }

    Network joinNetwork(string id, NETWORK_KEY_T key);
    Network joinLastNetwork();

    bool lastJoinedAvailable();
};


#endif //LUMOS_NETWORK_H
