#include "Registry.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif


random_device rd;
mt19937 rng(rd());
uniform_int_distribution<int> broadcastIntervalRange(REGISTRY_BROADCAST_INTERVAL_MIN, REGISTRY_BROADCAST_INTERVAL_MAX);

Registry::Registry(string name, map<PUB_HASH_T, Crypto::asym::PublicKey *> *keys, StorageHandler *stor,
                   NetworkHandler *net, REL_TIME_PROV_T relTimeProvider)
        : stor(stor), net(net), relTimeProvider(relTimeProvider),
          bcast(net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)),
          nextBroadcast(relTimeProvider->millis() + REGISTRY_BROADCAST_INTERVAL_MIN),
          name(name), instanceIdentifier(Crypto::generateUUID()), trustedKeys(keys) {
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
    this->headState.clear();
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
                this->headState[entry.key] = entry.value;
                break;
            case RegistryEntryType::DELETE:
                this->headState.erase(entry.key);
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

tuple<vector<unsigned long>, unsigned long> Registry::getBlockBorders(string parentUUID) {
    unsigned long i = this->entries.size();

    vector<unsigned long> blockBorders = {this->entries.size()};

    // 1. Search for block borders (=> locations where the parentUUID's don't match up)
    // 2. Search for the parent entry and quit at its location
    // TODO entries[i-1] might be causing memory leaks since the loop goes on until i=0 so i-1=-1 which is invalid
    while (--i && this->entries[i].uuid != parentUUID)
        if (this->entries[i - 1].uuid != this->entries[i].parentUUID) blockBorders.push_back(i);

    return make_tuple(blockBorders, i + 1);
}

void Registry::addEntry(RegistryEntry e, bool save) {
    unsigned long index = 0;

    // If the entry is supposed to have a parent then calculate its new position
    if (e.parentUUID.size() && this->entries.size() > 0) {
        auto res = this->getBlockBorders(e.parentUUID);
        vector<unsigned long> blockBorders(std::get<0>(res));
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

    // Check the entries neighbours to avoid dups
    if ( !( this->entries.size() > 0 && (
            (index > 0 && this->entries[index-1].uuid == e.uuid)
            || (index < (this->entries.size()-1) && this->entries[index+1].uuid == e.uuid)
                                        )
          )
    ) {
        // Insert the entry at the previously determined position
        this->entries.insert(this->entries.begin() + index, e);
        this->updateHead(save);
    }
}

string Registry::getHeadUUID() {
    if (this->entries.size() == 0) return "";
    return this->entries.back().uuid;
}

string Registry::get(string key) {
    auto i = this->headState.find(key);
    if (i != this->headState.end()) return i->second;
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
    auto i = this->headState.find(key);
    return i != this->headState.end();
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
    this->headState.clear();
}

void Registry::sync() {
    if (this->relTimeProvider->millis() < this->nextBroadcast ||
        !this->hashChain.size())
        return;
    this->nextBroadcast = this->relTimeProvider->millis() + broadcastIntervalRange(rng);

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();

    root["type"] = "REG_HEAD";
    root["registryName"] = this->name;
    root["head"] = this->hashChain.back();
    root["length"] = this->entries.size();
    root["instance"] = this->instanceIdentifier;

    string msg;
    root.printTo(msg);

    // DEBUG
//    Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
//    string newVal = "someNewValue";
//    newVal += this->relTimeProvider->millis();
//    this->set("someDevice", newVal, pair);

    std::cout << "OUTGOING SYNC REQUEST" << std::endl;
    // DONE DEBUG

    this->bcast->broadcast(msg);
}

UUID Registry::requestHash(double index, string target, UUID requestID) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &request = jsonBuffer.createObject();
    request["type"] = "GET_HASH";
    request["index"] = index;
    request["requestID"] = requestID;
    request["targetInstance"] = target;
    request["registryName"] = this->name;

    string requestStr;
    request.printTo(requestStr);
    this->bcast->broadcast(requestStr);

    return requestID;
}

bool Registry::isSyncInProgress() {
    return this->relTimeProvider->millis() - this->synchronizationStatus.lastRequestTimestamp < REGISTRY_SYNC_TIMEOUT;
}

void Registry::onData(string request) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.parse(request);
    string head = this->hashChain.size() > 0 ? this->hashChain.back() : "";

    if (root.success() && root.containsKey("registryName") && root["registryName"] == this->name && root.containsKey("type")) {

        if (root["type"] == "GET_HASH" && root["targetInstance"] == this->instanceIdentifier) {
            if (! (root.containsKey("index") && root.containsKey("requestID"))) return;
            JsonObject &hashBroadcast = jsonBuffer.createObject();
            hashBroadcast["type"] = "REG_HASH";
//            hashBroadcast["index"] = root["index"];
            hashBroadcast["answerID"] = root["requestID"];
            hashBroadcast["registryName"] = this->name;
            hashBroadcast["instance"] = this->instanceIdentifier;
            if (this->hashChain.size() > root["index"].as<double>())
                hashBroadcast["hash"] = this->hashChain[root["index"].as<double>()];
            else
                hashBroadcast["hash"] = "";

            string hashBcast;
            hashBroadcast.printTo(hashBcast);
            this->bcast->broadcast(hashBcast);
        }

        if (root["type"] == "REG_HEAD" && head != root["head"] && root.containsKey("instance") && !this->isSyncInProgress()) {
            // TODO SYNC STUFF
            cout << "SYNC" << endl;
            double l_remote = root["length"];
            double l_min = min(l_remote, (double) this->entries.size()-1);

            UUID requestID(Crypto::generateUUID());
            this->synchronizationStatus.lastRequestTimestamp = this->relTimeProvider->millis();
            this->synchronizationStatus.requestID = requestID;
            this->synchronizationStatus.min = 0;
            this->synchronizationStatus.max = l_min;
            this->requestHash(ceil(l_min/2), root["instance"], requestID);

        }

        if (root["type"] == "REG_HASH" && root.containsKey("hash") && root.containsKey("answerID") &&
            root["answerID"] == this->synchronizationStatus.requestID && this->isSyncInProgress()) {
            double min = this->synchronizationStatus.min;
            double max = this->synchronizationStatus.max;
            double index = ceil(min + (max-min)/2);
            cout << "step" << endl;

            if (this->hashChain.size() <= index) return;

            if (this->hashChain[index] == root["hash"])
                min = index+1;
            else
                max = index-1;

            if (min > max) {
                cout << "HORRAY we've found the index @ " << min << endl;
            } else if (min == index) {
                cout << "HORRAY we've found the index @ " << index << endl;
            } else {
                UUID requestID(Crypto::generateUUID());
                this->synchronizationStatus.lastRequestTimestamp = this->relTimeProvider->millis();
                this->synchronizationStatus.requestID = requestID;
                this->synchronizationStatus.min = min;
                this->synchronizationStatus.max = max;
                this->requestHash(ceil(min + (max-min)/2), root["instance"], requestID);
            }
        }
    }
}

string Registry::getHeadHash() const {
    return this->hashChain.back();
}

#ifdef UNIT_TESTING
    SCENARIO("Database/Registry", "[registry]") {
        GIVEN("a cleared registry and a KeyPair") {
            Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());

            map<PUB_HASH_T, Crypto::asym::PublicKey *> keys = { { pair.pub.getHash(), &pair.pub } };
            DummyNetworkHandler dnet;
            DummyStorageHandler dstor;
            REL_TIME_PROV_T drelTimeProv(new DummyRelativeTimeProvider);

            Registry reg("someRegistry", &keys, &dstor, &dnet, drelTimeProv);

            WHEN("a serialized entry is added twice") {
                reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"someintermediate\",\"parentUUID\":\"y\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
                reg.addSerializedEntry("{\"metadata\":{\"uuid\":\"someintermediate\",\"parentUUID\":\"y\",\"signature\":\"ebd6a67e627b02947d131706fd6e75344af1518621852a01f548744801005e09074363a5b795b882e70e80c75df86942cbf2a644a918f07b3566d8d8044fe119\",\"publicKeyUsed\":\"e591486d713f21f4\",\"type\":\"UPSERT\"},\"content\":{\"key\":\"someDevice\",\"value\":\"someValue\"}}", false);
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

            WHEN("a value is set with a different public key (not in the list of trusted keys)") {
                string key("someKey");
                string val("someValue");
                Crypto::asym::KeyPair otherPair(Crypto::asym::generateKeyPair());
                reg.set(key, val, otherPair);

                THEN("has should be false") {
                    REQUIRE_FALSE(reg.has(key));
                }

                THEN("the read value should be empty") {
                    REQUIRE( reg.get(key) == "" );
                }
            }
        }
    }
#endif