#ifndef OPENHOME_NETWORK_HPP
#define OPENHOME_NETWORK_HPP


#include "../api/network.hpp"
#include "../api/storage.hpp"
#include "../api/time.hpp"
#include "../registry/Registry.hpp"

class Network {
    NetworkProvider* net;
    StorageProvider* stor;
    REL_TIME_PROV_T time;

    Crypto::asym::PublicKey masterKey;

    map<string, Registry> registries;

    void loadRegistry(string name);
public:
    Network(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider, Crypto::asym::PublicKey masterKey);
};


#endif //OPENHOME_NETWORK_HPP
