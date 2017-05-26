#include "Registry.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

Registry::Registry(string name, map<PUB_HASH_T, Crypto::asym::PublicKey *> *keys, StorageHandler *stor,
                   NetworkHandler *net)
        : name(name), trustedKeys(keys), stor(stor), net(net) {
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
    this->hashChain.clear();

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& serializedEntries = jsonBuffer.createArray();

    string lastHash = "";

    for (auto &entry : entries) {
        // Save entries
        serializedEntries.add(string(entry));
        // Generate hash for the entry
        lastHash = Crypto::hash::sha512(lastHash + entry.getSignatureText());
        this->hashChain.push_back(lastHash);
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

std::tuple<vector<unsigned long>, unsigned long> Registry::getBlockBorders(string parentUUID) {
    unsigned long i = this->entries.size();

    vector<unsigned long> blockBorders = {this->entries.size()};

    // 1. Search for block borders (=> locations where the parentUUID's don't match up)
    // 2. Search for the parent entry and quit at its location
    // TODO entries[i-1] might be causing memory leaks since the loop goes on until i=0 so i-1=-1 which is invalid
    while (--i && this->entries[i].uuid != parentUUID)
        if (this->entries[i - 1].uuid != this->entries[i].parentUUID) blockBorders.push_back(i);

    return std::make_tuple(blockBorders, i + 1);
}

void Registry::addEntry(RegistryEntry e, bool save) {
    unsigned long index = 0;

    // If the entry is supposed to have a parent then calculate its new position
    if (e.parentUUID.size()) {
        auto res = this->getBlockBorders(e.parentUUID);
        vector<unsigned long> blockBorders = std::get<0>(res);
        index = std::get<1>(res);

        unsigned long lastBorder = blockBorders.back();
        if (lastBorder != index) {
            // Check if the insertion would create an additional border and compensate
            if (this->entries[index].parentUUID == e.parentUUID) blockBorders.push_back(index);

            index = this->entries.size();
            for (auto border = blockBorders.rbegin(); border != blockBorders.rend(); ++border) {
                // This is the actual "merge". The sorting is done by comparing the UUID's
                if (*border != this->entries.size() && e.uuid < this->entries[*border].uuid) {
                    index = *border;
                    break;
                }
            }
        }
    }

    // Insert the entry at the previously determined position
    this->entries.insert(this->entries.begin() + index, e);
    this->updateHead(save);
}

string Registry::getHeadUUID() {
    if (this->entries.size() == 0) return "";
    return this->entries.back().uuid;
}

std::string Registry::get(string key) {
    auto i = this->head.find(key);
    if (i != this->head.end()) return i->second;
    return "";
}

void Registry::set(string key, string value, Crypto::asym::KeyPair pair) {
    this->addEntry(
            RegistryEntry(RegistryEntryType::UPSERT, key, value, pair, this->getHeadUUID())
    );
}

void Registry::del(string key, Crypto::asym::KeyPair pair) {
    this->addEntry(
            RegistryEntry(RegistryEntryType::DELETE, key, "", pair, this->getHeadUUID())
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

void Registry::clear() {
    this->stor->set(this->name, "");
    this->entries.clear();
    this->hashChain.clear();
    this->head.clear();
}

void Registry::sync() {
    if (this->hashChain.size() == 0) return;

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root["type"] = "REG_HEAD";
    root["registryName"] = this->name;
    root["head"] = this->hashChain.back();

    string msg;
    root.printTo(msg);
    this->net->broadcast(msg);
    std::cout << "SYNC\n";

    // DEBUG
//    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
//    this->set("someDevice", "someNewValue", pair);
    this->onSyncRequest(msg);
}

void Registry::onSyncRequest(string request) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.parse(request);

    if (root["registryName"] != this->name) return;

    string head = this->hashChain.size() > 0 ? this->hashChain.back() : "";

    if (head != root["head"]) {
        // TODO SYNC STUFF
        std::cerr << "SYNC" << std::endl;
    }
}

#ifdef UNIT_TESTING
    SCENARIO("Database/Registry", "[entry]") {
        GIVEN("a cleared registry and a KeyPair") {
            Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
//            Registry reg("someRegistry", )
        }
    }
#endif