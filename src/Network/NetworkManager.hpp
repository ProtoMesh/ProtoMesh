#ifndef LUMOS_NETWORK_H
#define LUMOS_NETWORK_H

#include "../api/api.hpp"
#include "../api/time.hpp"
#include "../api/network.hpp"
#include "../api/storage.hpp"
#include "../Registry/Registry.hpp"
#include "../crypto/crypto.hpp"
#include "Network.hpp"

class NetworkManager {
    APIProvider api;
public:
    NetworkManager(shared_ptr<NetworkProvider> net, shared_ptr<StorageProvider> stor, shared_ptr<RelativeTimeProvider> relTimeProvider)
            : api{NULL, stor, net, relTimeProvider} {};

    Crypto::asym::KeyPair createNetwork() {
        // TODO Initialize the data in the registries for that network
        return Crypto::asym::generateKeyPair();
    }

    Network joinNetwork(string id, NETWORK_KEY_T key);
    Network joinLastNetwork();

    bool lastJoinedAvailable();
};


#endif //LUMOS_NETWORK_H
