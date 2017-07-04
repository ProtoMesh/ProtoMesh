#include "Registry.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif


random_device rd;
mt19937 rng(rd());
uniform_int_distribution<int> broadcastIntervalRange(REGISTRY_BROADCAST_INTERVAL_MIN, REGISTRY_BROADCAST_INTERVAL_MAX);

template <typename VALUE_T>
Registry<VALUE_T>::Registry(string name, StorageProvider *stor, NetworkProvider *net, REL_TIME_PROV_T relTimeProvider)
        : stor(stor), net(net), relTimeProvider(relTimeProvider),
          bcast(net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)),
          nextBroadcast(relTimeProvider->millis() + REGISTRY_BROADCAST_INTERVAL_MIN),
          name(name), instanceIdentifier(Crypto::generateUUID()) {
    using namespace openHome::registry;

    this->synchronizationStatus.lastRequestTimestamp = this->relTimeProvider->millis() - REGISTRY_SYNC_TIMEOUT;

    if (this->stor->has(REGISTRY_STORAGE_PREFIX + this->name)) {
        vector<uint8_t> serializedRegistry(this->stor->get(REGISTRY_STORAGE_PREFIX + this->name));
        auto registry = GetRegistry(serializedRegistry.data());
        registry->entries()->Length();

        for (flatbuffers::uoffset_t i = 0; i < registry->entries()->Length(); i++) {
            this->addSerializedEntry(registry->entries()->Get(i), false);
        }
    }

    cout << "initialized Registry: " << name << endl;
}

template <typename VALUE_T>
void Registry<VALUE_T>::updateHead(bool save) {
    using namespace openHome::registry;

    this->headState.clear();
    this->hashChain.clear();

    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<Entry>> entryOffsets;

    string lastHash = "";

    for (auto &entry : entries) {
        // Save entries
        entryOffsets.push_back(entry.to_flatbuffer_offset(builder));
        // Generate hash for the entry
        lastHash = Crypto::hash::sha512(lastHash + entry.getSignatureText());
        this->hashChain.push_back(lastHash);
        // Update head state
//        if (entry.verifySignature(this->trustedKeys) != RegistryEntry::Verify::OK) continue; /// TODO Verify signature and add permission check
        switch (entry.type) {
            case RegistryEntryType::UPSERT:
                this->headState[entry.key] = entry.value;
                break;
            case RegistryEntryType::DELETE:
                this->headState.erase(entry.key);
                break;
        }
    }

    if (save) {
        auto entries = builder.CreateVector(entryOffsets);
        auto registry = CreateRegistry(builder, entries);
        builder.Finish(registry);

        uint8_t *buf = builder.GetBufferPointer();
        vector<uint8_t> serializedRegistry(buf, buf + builder.GetSize());
        this->stor->set(REGISTRY_STORAGE_PREFIX + this->name, serializedRegistry);
    }
}

template <typename VALUE_T>
tuple<vector<unsigned long>, unsigned long> Registry<VALUE_T>::getBlockBorders(Crypto::UUID parentUUID) {
    unsigned long i = this->entries.size();

    vector<unsigned long> blockBorders = {this->entries.size()};

    // 1. Search for block borders (=> locations where the parentUUID's don't match up)
    // 2. Search for the parent entry and quit at its location
    // TODO entries[i-1] might be causing memory leaks since the loop goes on until i=0 so i-1=-1 which is invalid
    while (--i && this->entries[i].uuid != parentUUID)
        if (this->entries[i - 1].uuid != this->entries[i].parentUUID) blockBorders.push_back(i);

    return make_tuple(blockBorders, i + 1);
}

template <typename VALUE_T>
bool Registry<VALUE_T>::addEntry(RegistryEntry<VALUE_T> e, bool save) {
    if (!e.valid) return false;
    unsigned long index = 0;

    // If the entry is supposed to have a parent then calculate its new position
    if (e.parentUUID != Crypto::UUID::Empty() && this->entries.size() > 0) {
        auto res = this->getBlockBorders(e.parentUUID);
        vector<unsigned long> blockBorders(std::get<0>(res));
        index = std::get<1>(res);

        unsigned long lastBorder = blockBorders.back();
        if (lastBorder != index) {
            // Check if the insertion would create an additional border and compensate
            if (this->entries[index].parentUUID == e.parentUUID) blockBorders.push_back(index);

            index = this->entries.size();
            for (auto border = blockBorders.rbegin(); border != blockBorders.rend(); ++border) {

                // If the entry is already present then don't create a duplicate
                if (this->entries.size() > *border && this->entries[*border].uuid == e.uuid) return false;

                // This is the actual "merge". The sorting is done by comparing the UUID's
                if (*border != this->entries.size() && e.uuid < this->entries[*border].uuid) {
                    index = *border;
                    break;
                }
            }
        }
    }


    bool below_identical = false, above_identical = false, target_identical = false;
    if (this->entries.size() > 0) {
        below_identical = (index > 0 && this->entries[index - 1].uuid == e.uuid);
        above_identical = (index < (this->entries.size() - 1) && this->entries[index + 1].uuid == e.uuid);
        target_identical = (this->entries[index].uuid == e.uuid);
    }

    // Check the entries neighbours to avoid dups
    if (!below_identical && !above_identical && !target_identical) {
        // Insert the entry at the previously determined position
        this->entries.insert(this->entries.begin() + index, e);
        this->updateHead(save);
        return true;
    }

    return false;
}

template <typename VALUE_T>
Crypto::UUID Registry<VALUE_T>::getHeadUUID() {
    if (this->entries.size() == 0) return Crypto::UUID::Empty();
    return this->entries.back().uuid;
}

template <typename VALUE_T>
string Registry<VALUE_T>::get(string key) {
    auto i = this->headState.find(key);
    if (i != this->headState.end()) return i->second;
    return "";
}

template <typename VALUE_T>
void Registry<VALUE_T>::set(string key, string value, Crypto::asym::KeyPair pair) {
    this->addEntry(
            RegistryEntry<VALUE_T>(RegistryEntryType::UPSERT, key, value, pair, this->getHeadUUID())
    );
}

template <typename VALUE_T>
void Registry<VALUE_T>::del(string key, Crypto::asym::KeyPair pair) {
    this->addEntry(
            RegistryEntry<VALUE_T>(RegistryEntryType::DELETE, key, "", pair, this->getHeadUUID())
    );
}

template <typename VALUE_T>
bool Registry<VALUE_T>::has(string key) {
    auto i = this->headState.find(key);
    return i != this->headState.end();
}

template <typename VALUE_T>
bool Registry<VALUE_T>::addSerializedEntry(const openHome::registry::Entry* serialized, bool save) {
    return this->addEntry(RegistryEntry<VALUE_T>(serialized), save);
}

template <typename VALUE_T>
void Registry<VALUE_T>::clear() {
    this->stor->set(this->name, "");
    this->entries.clear();
    this->hashChain.clear();
    this->headState.clear();
}

template <typename VALUE_T>
void Registry<VALUE_T>::sync() {
    if (this->relTimeProvider->millis() < this->nextBroadcast ||
        !this->hashChain.size())
        return;
    this->nextBroadcast = this->relTimeProvider->millis() + broadcastIntervalRange(rng);

//    DynamicJsonBuffer jsonBuffer;
//    JsonObject &root = jsonBuffer.createObject();
//
//    root["type"] = "REG_HEAD";
//    root["registryName"] = this->name;
//    root["head"] = this->hashChain.back();
//    root["length"] = this->entries.size();
//    root["instance"] = this->instanceIdentifier;
//
//    string msg;
//    root.printTo(msg);
//
//    this->bcast->broadcast(msg);
}

template <typename VALUE_T>
bool Registry<VALUE_T>::isSyncInProgress() {
    return this->relTimeProvider->millis() - this->synchronizationStatus.lastRequestTimestamp < REGISTRY_SYNC_TIMEOUT;
}

template <typename VALUE_T>
Crypto::UUID Registry<VALUE_T>::requestHash(size_t index, string target, Crypto::UUID requestID) {
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject &request = jsonBuffer.createObject();
//    request["type"] = "REG_GET_HASH";
//    request["index"] = index;
//    request["requestID"] = string(requestID);
//    request["targetInstance"] = target;
//    request["registryName"] = this->name;

//    string requestStr;
//    request.printTo(requestStr);
//    this->bcast->broadcast(requestStr);

    return requestID;
}

template <typename VALUE_T>
void Registry<VALUE_T>::broadcastEntries(size_t index) { // includes index

    for (size_t i = index; i < this->entries.size(); i++) {
//        DynamicJsonBuffer jsonBuffer;
//        JsonObject &request = jsonBuffer.createObject();
//
//        request["type"] = "REG_ENTRY";
//        request["registryName"] = this->name;
//        request["entry"] = (string) this->entries[i]; // TODO Implement
//        request["instance"] = this->instanceIdentifier;

//        string requestStr;
//        request.printTo(requestStr);
//        this->bcast->broadcast(requestStr);
    }
}

template <typename VALUE_T>
void Registry<VALUE_T>::onBinarySearchResult(size_t index) {
    this->broadcastEntries(index);

//    DynamicJsonBuffer jsonBuffer;
//    JsonObject &request = jsonBuffer.createObject();
//
//    request["type"] = "REG_REQUEST_ENTRIES";
//    request["registryName"] = this->name;
//    request["targetInstance"] = string(this->synchronizationStatus.communicationTarget);
//    request["index"] = index;
//
//    string requestStr;
//    request.printTo(requestStr);
//    this->bcast->broadcast(requestStr);
}

template <typename VALUE_T>
void Registry<VALUE_T>::onData(string incomingData) {
//    DynamicJsonBuffer jsonBuffer;
//    JsonObject &parsedData = jsonBuffer.parse(incomingData);
//    string head = this->hashChain.size() > 0 ? this->hashChain.back() : "";
//
//    if (parsedData.success() && parsedData.containsKey("registryName") && parsedData["registryName"] == this->name && parsedData.containsKey("type")) {
//
//        string type = parsedData["type"];
//
//        if (parsedData.containsKey("targetInstance") && parsedData["targetInstance"] == this->instanceIdentifier) {
//            if (type == "REG_REQUEST_ENTRIES" && parsedData.containsKey("index")) {
//                this->broadcastEntries(parsedData["index"]);
//            }
//
//            if (type == "REG_GET_HASH") {
//                if (! (parsedData.containsKey("index") && parsedData.containsKey("requestID"))) return;
//                JsonObject &hashBroadcast = jsonBuffer.createObject();
//                hashBroadcast["type"] = "REG_HASH";
//                hashBroadcast["answerID"] = parsedData["requestID"];
//                hashBroadcast["registryName"] = this->name;
//                hashBroadcast["instance"] = this->instanceIdentifier;
//                if (this->hashChain.size() > parsedData["index"].as<size_t>())
//                    hashBroadcast["hash"] = this->hashChain[parsedData["index"].as<size_t>()];
//                else
//                    hashBroadcast["hash"] = "";
//
//                string hashBcast;
//                hashBroadcast.printTo(hashBcast);
//                this->bcast->broadcast(hashBcast);
//            }
//        }
//
//        if (type == "REG_HEAD" && head != parsedData["head"] && parsedData.containsKey("instance") && !this->isSyncInProgress()) {
//            size_t l_remote = parsedData["length"];
//            size_t l_min = min(l_remote, (size_t) this->entries.size()-1);
//
//            Crypto::UUID requestID();
//            // TODO Implement commented out code
//            this->synchronizationStatus.lastRequestTimestamp = this->relTimeProvider->millis();
////            this->synchronizationStatus.requestID = requestID;
//            this->synchronizationStatus.min = 0;
//            this->synchronizationStatus.max = l_min;
////            this->synchronizationStatus.communicationTarget = parsedData["instance"].as<string>();
////            this->requestHash((size_t) ceil(l_min / 2), parsedData["instance"], requestID);
//
//        }
//
//        if (type == "REG_HASH" && parsedData.containsKey("hash") && parsedData.containsKey("answerID") &&
//            parsedData["answerID"] == this->synchronizationStatus.requestID && this->isSyncInProgress()) {
//            size_t min = this->synchronizationStatus.min;
//            size_t max = this->synchronizationStatus.max;
//            size_t index = (size_t) ceil(min + (max - min) / 2);
//
//            if (this->hashChain.size() <= index) return;
//
//            if (this->hashChain[index] == parsedData["hash"])
//                min = index+1;
//            else
//                max = index-1;
//
//            if (min > max) {
//                this->onBinarySearchResult(min);
//            } else if (min == index) {
//                this->onBinarySearchResult(index);
//            } else {
//                Crypto::UUID requestID();
//                // TODO Implement commented out code
//                this->synchronizationStatus.lastRequestTimestamp = this->relTimeProvider->millis();
////                this->synchronizationStatus.requestID = string(requestID);
//                this->synchronizationStatus.min = min;
//                this->synchronizationStatus.max = max;
////                this->requestHash((size_t) ceil(min + (max - min) / 2), parsedData["instance"], requestID);
//            }
//        }
//
//        if (type == "REG_ENTRY" && parsedData.containsKey("entry") && parsedData.containsKey("instance") && parsedData["instance"] != this->instanceIdentifier) {
//            cout << "[" << this->instanceIdentifier << "] INSERTING ENTRY" << endl;
//            this->addSerializedEntry(parsedData["entry"].as<string>());
////            cout << this->addSerializedEntry(parsedData["entry"].as<string>()) << " | " << this->entries.size() << endl;
//        }
//    }
}

template <typename VALUE_T>
string Registry<VALUE_T>::getHeadHash() const {
    if (this->hashChain.size() > 0)
        return this->hashChain.back();
    return "";
}

template class Registry<string>;

#ifdef UNIT_TESTING
    #include "flatbuffers/idl.h"

    SCENARIO("Database/Registry", "[registry]") {
        GIVEN("a cleared registry and a KeyPair") {
            Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());

            map<PUB_HASH_T, Crypto::asym::PublicKey *> keys = { { pair.pub.getHash(), &pair.pub } };
            DummyNetworkHandler dnet;
            DummyStorageHandler dstor;
            REL_TIME_PROV_T drelTimeProv(new DummyRelativeTimeProvider);

            Registry<string> reg("someRegistry", &dstor, &dnet, drelTimeProv);

            WHEN("a serialized entry is added twice") {

                std::string schemafile;
                std::string jsonfile;
                bool ok = flatbuffers::LoadFile("../src/buffers/registry/entry.fbs", false, &schemafile) &&
                          flatbuffers::LoadFile("../src/test/data/registry_entry.json", false, &jsonfile);
                REQUIRE(ok);

                flatbuffers::Parser parser;
                const char *include_directories[] = { "../src/buffers/", "../src/buffers/registry", nullptr };
                ok = parser.Parse(schemafile.c_str(), include_directories) &&
                     parser.Parse(jsonfile.c_str(), include_directories);

                CAPTURE(parser.error_);
                REQUIRE(ok);

                auto entry = openHome::registry::GetEntry(parser.builder_.GetBufferPointer());
                reg.addSerializedEntry(entry);
                reg.addSerializedEntry(entry);

                THEN("the second one should be omitted") {
                    REQUIRE(reg.entries.size() == 1);
                }
            }

            WHEN("a value is set") {
                string key("someKey");
                string val("someValue");
                reg.set(key, val, pair);
                string prevHeadHash(reg.getHeadHash());

                THEN("has should be true") {
                    REQUIRE(reg.has(key));
                }
                THEN("the read value should be equal") {
                    REQUIRE( reg.get(key) == val );
                }

                AND_WHEN("the registry is cleared") {
                    reg.clear();

                    THEN("the read value should be empty") {
                        REQUIRE( reg.get(key) == "" );
                    }
                }

                AND_WHEN("the value is deleted") {
                    reg.del(key, pair);

                    THEN("the read value should be empty") {
                        REQUIRE( reg.get(key) == "" );
                    }

                    THEN("the head hash should differ") {
                        REQUIRE_FALSE( reg.getHeadHash() == prevHeadHash );
                    }
                }
            }

//            WHEN("a value is set with a different public key (not in the list of trusted keys)") {
//                string key("someKey");
//                string val("someValue");
//                Crypto::asym::KeyPair otherPair(Crypto::asym::generateKeyPair());
//                reg.set(key, val, otherPair);
//
//                THEN("has should be false") {
//                    REQUIRE_FALSE(reg.has(key));
//                }
//
//                THEN("the read value should be empty") {
//                    REQUIRE( reg.get(key) == "" );
//                }
//            }
        }
    }
#endif
