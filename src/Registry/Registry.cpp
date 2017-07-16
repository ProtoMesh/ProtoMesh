#include "Registry.hpp"

#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

template class Registry<vector<uint8_t>>;

template <typename VALUE_T>
Registry<VALUE_T>::Registry(APIProvider api, string name, string validator)
    : api(api),
          bcast(api.net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)),
          name(name), instanceIdentifier(), validator(validator),
          nextBroadcast(api.time->millis() + REGISTRY_BROADCAST_INTERVAL_MIN) {
    using namespace lumos::registry;

    this->synchronizationStatus.lastRequestTimestamp = this->api.time->millis() - REGISTRY_SYNC_TIMEOUT;

    if (this->api.stor->has(REGISTRY_STORAGE_PREFIX + this->name)) {
        vector<uint8_t> serializedRegistry(this->api.stor->get(REGISTRY_STORAGE_PREFIX + this->name));
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
HASH Registry<VALUE_T>::getHeadHash() const {
    if (this->hashChain.size() > 0)
        return this->hashChain.back();
    return {};
}

/// ------------------------------------------ High level data manipulation -------------------------------------------

template <typename VALUE_T>
VALUE_T Registry<VALUE_T>::get(string key) {
    auto i = this->headState.find(key);
    if (i != this->headState.end()) return i->second;
    return {};
}

template <typename VALUE_T>
RegistryModificationResult Registry<VALUE_T>::set(string key, VALUE_T value, Crypto::asym::KeyPair pair) {
    if (this->has(key) && this->get(key) == value) return RegistryModificationResult::AlreadyPresent;
    return this->addEntry(
            RegistryEntry<VALUE_T>(RegistryEntryType::UPSERT, key, value, pair, this->getHeadUUID())
    );
}

template <typename VALUE_T>
RegistryModificationResult Registry<VALUE_T>::del(string key, Crypto::asym::KeyPair pair) {
    if (!this->has(key)) return RegistryModificationResult::AlreadyPresent;
    return this->addEntry(
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
    this->api.stor->set(this->name, "");
    this->entries.clear();
    this->hashChain.clear();
    this->headState.clear();
}


template <typename VALUE_T>
void Registry<VALUE_T>::onChange(std::function<void(RegistryEntry<vector<uint8_t>>)> listener) {
    this->listeners.push_back(listener);
}