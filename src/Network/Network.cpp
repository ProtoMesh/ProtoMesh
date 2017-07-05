#include "Network.hpp"

Network::Network(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider, Crypto::asym::PublicKey masterKey) : net(net), stor(stor), time(relTimeProvider), masterKey(masterKey) {
    // TODO load groups
    this->loadRegistry("groups");
    // TODO load devices
}

void Network::loadRegistry(string name) {
//    std::map<PUB_HASH_T, Crypto::asym::PublicKey *> trustedKeys;
//    trustedKeys[this->masterKey.getHash()] = &this->masterKey;

    Registry<vector<uint8_t>> reg(name, this->stor, this->net, this->time);
    this->registries.emplace(name, reg);
}
