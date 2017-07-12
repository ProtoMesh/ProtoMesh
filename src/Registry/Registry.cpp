#include "Registry.hpp"

#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

template class Registry<vector<uint8_t>>;

template <typename VALUE_T>
Registry<VALUE_T>::Registry(string name, StorageProvider *stor, NetworkProvider *net, REL_TIME_PROV_T relTimeProvider, string validator)
        : stor(stor), net(net), relTimeProvider(relTimeProvider),
          bcast(net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)),
          name(name), instanceIdentifier(), validator(validator),
          nextBroadcast(relTimeProvider->millis() + REGISTRY_BROADCAST_INTERVAL_MIN) {
    using namespace lumos::registry;

    this->synchronizationStatus.lastRequestTimestamp = this->relTimeProvider->millis() - REGISTRY_SYNC_TIMEOUT;

    if (this->stor->has(REGISTRY_STORAGE_PREFIX + this->name)) {
        vector<uint8_t> serializedRegistry(this->stor->get(REGISTRY_STORAGE_PREFIX + this->name));
        auto registry = GetRegistry(serializedRegistry.data());
        registry->entries()->Length();

        for (flatbuffers::uoffset_t i = 0; i < registry->entries()->Length(); i++) {
            this->addSerializedEntry(registry->entries()->Get(i), false);
        }
    }
}

template <typename VALUE_T>
Crypto::UUID Registry<VALUE_T>::getHeadUUID() {
    if (this->entries.size() == 0) return Crypto::UUID::Empty();
    return this->entries.back().uuid;
}

template <typename VALUE_T>
string Registry<VALUE_T>::getHeadHash() const {
    if (this->hashChain.size() > 0)
        return this->hashChain.back();
    return "";
}

/// ------------------------------------------ High level data manipulation -------------------------------------------

template <typename VALUE_T>
VALUE_T Registry<VALUE_T>::get(string key) {
    auto i = this->headState.find(key);
    if (i != this->headState.end()) return i->second;
    return {};
}

template <typename VALUE_T>
void Registry<VALUE_T>::set(string key, VALUE_T value, Crypto::asym::KeyPair pair) {

    auto entry = RegistryEntry<VALUE_T>(RegistryEntryType::UPSERT, key, value, pair, this->getHeadUUID());
    this->addEntry(entry);
}

template <typename VALUE_T>
void Registry<VALUE_T>::del(string key, Crypto::asym::KeyPair pair) {
    this->addEntry(
            RegistryEntry<VALUE_T>(RegistryEntryType::DELETE, key, {}, pair, this->getHeadUUID())
    );
}

template <typename VALUE_T>
bool Registry<VALUE_T>::has(string key) {
    auto i = this->headState.find(key);
    return i != this->headState.end();
}

template <typename VALUE_T>
void Registry<VALUE_T>::clear() {
    this->stor->set(this->name, "");
    this->entries.clear();
    this->hashChain.clear();
    this->headState.clear();
}
