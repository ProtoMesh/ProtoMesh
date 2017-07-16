#include "Network.hpp"

Network::Network(APIProvider api, Crypto::asym::PublicKey masterKey)
    : api(api), masterKey(masterKey), registryBcast(api.net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)) {
    using namespace std::placeholders;

    this->loadRegistry(NODES_REGISTRY)->onChange(std::bind(&Network::onNodeRegistryChange, this, _1));
    this->loadRegistry(GROUPS_REGISTRY)->onChange(std::bind(&Network::onGroupRegistryChange, this, _1));
}

STORED_REGISTRY_T Network::loadRegistry(string name) {
    auto sp = std::make_shared<Registry<vector<uint8_t>>>(api, name);

    sp->clear(); // TODO Remove this

    this->registries.emplace(name, sp);
    return sp;
}

STORED_REGISTRY_T Network::getRegistry(string name) {
    auto registry = this->registries.find(name);
    if (registry == this->registries.end()) return nullptr;
    return registry->second;
}

void Network::tick(unsigned int timeoutMS) {
    /// Receive data && let the registries broadcast their stuff
    vector<uint8_t> registryData;
    if (this->registryBcast->recv(&registryData, timeoutMS / 2) == RECV_OK) {
        for (auto reg : this->registries) {
            STORED_REGISTRY_T r = reg.second;
            r->onData(registryData);
            r->sync();
        }
    } else for (auto reg : this->registries) reg.second->sync();
}