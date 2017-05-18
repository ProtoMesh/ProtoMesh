#include "Registry.hpp"

Registry::Registry(string name, map<PUB_HASH_T, Crypto::asym::PublicKey*>* keys, StorageHandler* stor)
        : name(name), trustedKeys(keys), stor(stor) {
    if (this->stor->has(this->name)) {
        DynamicJsonBuffer jsonBuffer;
        string loadedRegistry(this->stor->get(this->name));
        JsonObject& root = jsonBuffer.parse(loadedRegistry);
        JsonArray& entries = root["entries"];
        for (auto entry : entries) {
            this->addSerializedEntry(entry, false);
        }
    }
}

void Registry::updateHead(bool save) {
    this->head.clear();

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& serializedEntries = jsonBuffer.createArray();

    for (auto &entry : entries) {
        // Save entries
        serializedEntries.add(entry.serialize());
        // Update head state
        if (entry.verifySignature(*this->trustedKeys) != RegistryEntry::Verify::OK) continue;
        switch (entry.type) {
            case RegistryEntryType::UPSERT:
                this->head[entry.key] = entry.value;
                break;
            case RegistryEntryType::DELETE:
                this->head.erase(entry.key);
                break;
        }
    }

    if (save) {
        root["entries"] = serializedEntries;
        string serializedRegistry;
        root.printTo(serializedRegistry);
        this->stor->set(this->name, serializedRegistry);
    }
}

void Registry::addEntry(RegistryEntry e, bool save) {
    std::cout << e.serialize() << std::endl;
    this->entries.push_back(e);
    this->updateHead(save);
}

std::string Registry::get(string key) {
    auto i = this->head.find(key);
    if (i != this->head.end()) return i->second;
    return "";
}

void Registry::set(string key, string value, Crypto::asym::KeyPair pair) {
    this->addEntry(
            RegistryEntry(RegistryEntryType::UPSERT, key, value, pair)
    );
}

void Registry::del(string key, Crypto::asym::KeyPair pair) {
    this->addEntry(
            RegistryEntry(RegistryEntryType::DELETE, key, "", pair)
    );
}

bool Registry::has(string key) {
    auto i = this->head.find(key);
    return i != this->head.end();
}

void Registry::addSerializedEntry(string serialized, bool save) {
    this->addEntry(RegistryEntry(serialized), save);
}

void Registry::setTrustedKeys(map<PUB_HASH_T, Crypto::asym::PublicKey*>* keys) {
    this->trustedKeys = keys;
}

void Registry::storeRegistry() {

}