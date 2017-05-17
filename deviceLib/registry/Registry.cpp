#include "Registry.hpp"

void Registry::updateHead() {
    this->head.clear();
    for (auto &entry : entries) {
        if (entry.verifySignature(this->trustedKeys) != RegistryEntry::Verify::OK) continue;
        switch (entry.type) {
            case RegistryEntryType::UPSERT:
                this->head[entry.key] = entry.value;
                break;
            case RegistryEntryType::DELETE:
                this->head.erase(entry.key);
                break;
        }
    }
    this->headOutdated = false;
}

void Registry::addEntry(RegistryEntry e) {
    this->entries.push_back(e);
    this->headOutdated = true;
}

std::string Registry::get(string key) {
    if (this->headOutdated) this->updateHead();
    auto i = this->head.find(key);
    if (i != this->head.end()) return i->second;
    return "";
}

void Registry::set(string key, string value, Crypto::asymmetric::KeyPair pair) {
    this->addEntry(
            RegistryEntry(RegistryEntryType::UPSERT, key, value, pair)
    );
}

void Registry::del(string key, Crypto::asymmetric::KeyPair pair) {
    this->addEntry(
            RegistryEntry(RegistryEntryType::DELETE, key, "", pair)
    );
}

bool Registry::has(string key) {
    if (this->headOutdated) this->updateHead();
    auto i = this->head.find(key);
    return i != this->head.end();
}

void Registry::addSerializedEntry(string serialized) {
    this->addEntry(RegistryEntry(serialized));
}

void Registry::setTrustedKeys(map<PUB_HASH_T, PUBLIC_KEY_T> keys) {
    this->trustedKeys = keys;
}