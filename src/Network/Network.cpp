#include "Network.hpp"

Network::Network(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider, Crypto::asym::PublicKey masterKey)
        : net(net), stor(stor), time(relTimeProvider),
          masterKey(masterKey), registryBcast(net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)) {


    // TODO load groups
    this->loadRegistry("groups");
    // TODO load devices
}

void Network::loadRegistry(string name) {
//    std::map<PUB_HASH_T, Crypto::asym::PublicKey *> trustedKeys;
//    trustedKeys[this->masterKey.getHash()] = &this->masterKey;

//    Registry<vector<uint8_t>> reg(name, this->stor, this->net, this->time); // TODO Check whether or not this is deallocated...
    auto sp = std::make_shared<Registry<vector<uint8_t>>>(name, this->stor, this->net, this->time);
    this->registries.emplace(name, sp);
}

void Network::tick(unsigned int timeoutMS) {
    // Receive data && let the registries broadcast their stuff
    vector<uint8_t> registryData;
    if (this->registryBcast->recv(&registryData, timeoutMS / 2) == RECV_OK) {
        for (auto reg : this->registries) {
            STORED_REGISTRY_T r = reg.second;
            r->onData(registryData);
            r->sync();
        }
    } else {
        for (auto reg : this->registries) {
            STORED_REGISTRY_T r = reg.second;
            r->sync();
        }
    }
}
