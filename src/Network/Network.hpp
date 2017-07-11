#ifndef LUMOS_NETWORK_HPP
#define LUMOS_NETWORK_HPP


#include "../api/network.hpp"
#include "../api/storage.hpp"
#include "../api/time.hpp"
#include "../Registry/Registry.hpp"

#define STORED_REGISTRY_T std::shared_ptr<Registry<vector<uint8_t>>>

class Network {
    NetworkProvider* net;
    StorageProvider* stor;
    REL_TIME_PROV_T time;

    Crypto::asym::PublicKey masterKey;

public:
    map<string, STORED_REGISTRY_T> registries;
    BCAST_SOCKET_T registryBcast;

    void loadRegistry(string name);

    Network(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider, Crypto::asym::PublicKey masterKey);

    void tick(unsigned int timeoutMS);
};


#endif //LUMOS_NETWORK_HPP
