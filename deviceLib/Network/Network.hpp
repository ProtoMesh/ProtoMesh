//
// Created by themegatb on 6/15/17.
//

#ifndef UCL_NETWORK_H
#define UCL_NETWORK_H

#include "../api/time.hpp"
#include "../api/network.hpp"
#include "../api/storage.hpp"
#include "../registry/Registry.hpp"
#include "../crypto/crypto.hpp"

class Network {
    NetworkProvider* net;
    StorageProvider* stor;
    REL_TIME_PROV_T time;

//    map<string, Registry> registries;
public:
    Network(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider) : net(net), stor(stor), time(relTimeProvider) {};
};

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

//    Network joinNetwork(string id) {
//         // TODO load network key from storage or fail
//    }

    Network joinNetwork(string id, NETWORK_KEY_T key) {
        // TODO Store network key

        return Network(this->net, this->stor, this->time);
    }
};


#endif //UCL_NETWORK_H
