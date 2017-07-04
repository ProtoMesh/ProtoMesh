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
          name(name), instanceIdentifier() {
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
void Registry<VALUE_T>::sync(bool force) {
    // Check if a sync would be within the defined interval or too early
    if (!force && (this->relTimeProvider->millis() < this->nextBroadcast || !this->hashChain.size())) return;
    this->nextBroadcast = this->relTimeProvider->millis() + broadcastIntervalRange(rng);

    using namespace openHome::registry::sync;
    flatbuffers::FlatBufferBuilder builder;

    // Create all properties
    openHome::UUID id(this->instanceIdentifier.a, this->instanceIdentifier.b, this->instanceIdentifier.c, this->instanceIdentifier.d);
    auto name = builder.CreateString(this->name);
    auto headStr = builder.CreateString(this->getHeadHash());

    // Create the head buffer
    auto head = CreateHead(builder, name, &id, headStr, this->entries.size());

    // Convert it to a byte array
    builder.Finish(head, HeadIdentifier());
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> head_vec(buf, buf + builder.GetSize());

    // Broadcast the head
    this->bcast->broadcast(head_vec);
}

template <typename VALUE_T>
bool Registry<VALUE_T>::isSyncInProgress() {
    return this->relTimeProvider->millis() - this->synchronizationStatus.lastRequestTimestamp < REGISTRY_SYNC_TIMEOUT;
}

template <typename VALUE_T>
Crypto::UUID Registry<VALUE_T>::requestHash(size_t index, Crypto::UUID target, Crypto::UUID requestID) {
    using namespace openHome::registry::sync;
    flatbuffers::FlatBufferBuilder builder;

    // Create all request properties
    openHome::UUID rid(requestID.a, requestID.b, requestID.c, requestID.d);
    openHome::UUID tid(target.a, target.b, target.c, target.d);

    // Create the request
    auto request = CreateRequest(builder, RequestType_HASH, &tid, &rid, index);

    // Convert it to a byte array
    builder.Finish(request, RequestIdentifier());
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> request_vec(buf, buf + builder.GetSize());

    // Broadcast the head
    this->bcast->broadcast(request_vec);

    return requestID;
}

template <typename VALUE_T>
void Registry<VALUE_T>::broadcastEntries(size_t index) { // includes index
    using namespace openHome::registry::sync;

    for (size_t i = index; i < this->entries.size(); i++) {
        flatbuffers::FlatBufferBuilder builder;

        // Create all request properties
        auto name = builder.CreateString(this->name);
        openHome::UUID instance(this->instanceIdentifier.a, this->instanceIdentifier.b, this->instanceIdentifier.c, this->instanceIdentifier.d);
        auto reg_entry = this->entries[i].to_flatbuffer_offset(builder);

        // Create the entry
        auto entry = CreateEntry(builder, name, &instance, reg_entry);

        // Convert it to a byte array
        builder.Finish(entry, EntryIdentifier());
        uint8_t *buf = builder.GetBufferPointer();
        vector<uint8_t> entry_vec(buf, buf + builder.GetSize());

        // Broadcast the entry
        this->bcast->broadcast(entry_vec);
    }
}

template <typename VALUE_T>
void Registry<VALUE_T>::onBinarySearchResult(size_t index) {
    this->broadcastEntries(index);

    using namespace openHome::registry::sync;
    flatbuffers::FlatBufferBuilder builder;

    // Create all request properties
    Crypto::UUID target(this->synchronizationStatus.communicationTarget);
    openHome::UUID tid(target.a, target.b, target.c, target.d);

    // Create the request
    RequestBuilder requestBuilder(builder);

    requestBuilder.add_type(RequestType_ENTRIES);
    requestBuilder.add_target(&tid);
    requestBuilder.add_index(index);

    auto request = requestBuilder.Finish();

    // Convert it to a byte array
    builder.Finish(request, RequestIdentifier());
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> request_vec(buf, buf + builder.GetSize());

    // Broadcast the head
    this->bcast->broadcast(request_vec);
}

template <typename VALUE_T>
void Registry<VALUE_T>::onData(vector<uint8_t> incomingData) {
    using namespace openHome::registry::sync;

    // Generic lambdas and useful variables
    uint8_t* data = incomingData.data();
    auto verifier = flatbuffers::Verifier(data, incomingData.size());
    auto hasIdentifier = [&] (const char * id) -> bool { return flatbuffers::BufferHasIdentifier(data, id); };
    auto onVerifyFail = [] () { cerr << "[REG|SYNC] Received invalid buffer" << endl; };

    // Lambdas regarding sync stages
    // // Request related
    auto onEntriesRequest = [&] (const Request* req) {
        if (Crypto::UUID(req->target()) == this->instanceIdentifier)
            this->broadcastEntries(req->index());
    };
    auto onHashRequest = [&] (const Request* req) {
        if (Crypto::UUID(req->target()) == this->instanceIdentifier) {
            flatbuffers::FlatBufferBuilder builder;

            // Create all hash properties
            auto answerID = openHome::UUID(*req->requestID());
            openHome::UUID instance(this->instanceIdentifier.a, this->instanceIdentifier.b, this->instanceIdentifier.c, this->instanceIdentifier.d);
            auto hashStr = builder.CreateString(this->hashChain.size() > req->index() ? this->hashChain[req->index()] : "");

            // Create the hash
            auto hash = CreateHash(builder, &answerID, &instance, hashStr);

            // Convert it to a byte array
            builder.Finish(hash, HashIdentifier());
            uint8_t *buf = builder.GetBufferPointer();
            vector<uint8_t> hash_vec(buf, buf + builder.GetSize());

            // Broadcast the head
            this->bcast->broadcast(hash_vec);
        }
    };
    // // Head related
    auto onHead = [&] (const Head* req) {
        if (!this->isSyncInProgress() && req->head()->str() != this->getHeadHash()) {
            // We got a potential candidate for synchronization. Start the binary search!
            size_t l_remote = req->length();
            size_t l_min = min(l_remote, (size_t) this->entries.size()-1);

            Crypto::UUID requestID;
            this->synchronizationStatus.lastRequestTimestamp = this->relTimeProvider->millis();
            this->synchronizationStatus.requestID = requestID;
            this->synchronizationStatus.min = 0;
            this->synchronizationStatus.max = l_min;
            this->synchronizationStatus.communicationTarget = Crypto::UUID(req->instance());
            this->requestHash((size_t) ceil(l_min / 2), this->synchronizationStatus.communicationTarget, requestID);
        }
    };
    // // Entry related
    auto onEntry = [&] (const Entry* entry) {
        if (entry->name()->str() == this->name)
            this->addSerializedEntry(entry->entry());
    };
    // // Hash related
    auto onHash = [&] (const Hash* hash) {
        if (Crypto::UUID(hash->answerID()) == this->synchronizationStatus.requestID && this->isSyncInProgress()) {
            size_t min = this->synchronizationStatus.min;
            size_t max = this->synchronizationStatus.max;
            size_t index = (size_t) ceil(min + (max - min) / 2);

            // The local database does not contain any value beyond the current point (happens when this database is smaller than the other one)
            if (this->hashChain.size() <= index) {
                this->onBinarySearchResult(index);
                return;
            }

            if (this->hashChain[index] == hash->hash()->str())
                min = index+1;
            else
                max = index-1;

            if (min > max) {
                this->onBinarySearchResult(min);
            } else if (min == index) {
                this->onBinarySearchResult(index);
            } else {
                Crypto::UUID requestID;
                this->synchronizationStatus.lastRequestTimestamp = this->relTimeProvider->millis();
                this->synchronizationStatus.requestID = requestID;
                this->synchronizationStatus.min = min;
                this->synchronizationStatus.max = max;
                this->requestHash((size_t) ceil(min + (max - min) / 2), hash->instance(), requestID);
            }
        }
    };

    // Actual evaluation of incoming data
    // // Request ('RREQ')
    if (hasIdentifier(RequestIdentifier())) {
        if (!VerifyRequestBuffer(verifier)) { onVerifyFail(); return; }
        auto request = GetRequest(data);

        switch (request->type()) {
            case RequestType_ENTRIES:
                onEntriesRequest(request);
                break;
            case RequestType_HASH:
                onHashRequest(request);
                break;
        }
    }
    // // Head state ('RHED')
    else if (hasIdentifier(HeadIdentifier())) {
        if (!VerifyHeadBuffer(verifier)) { onVerifyFail(); return; }
        onHead(GetHead(data));
    }
    // // Encapsuled registry entry ('RSEN')
    else if (hasIdentifier(EntryIdentifier())) {
        if (!VerifyEntryBuffer(verifier)) { onVerifyFail(); return; }
        onEntry(GetEntry(data));
    }
    // // Entry/chain hash ('RHSH')
    else if (hasIdentifier(HashIdentifier())) {
        if (!VerifyHashBuffer(verifier)) { onVerifyFail(); return; }
        onHash(GetHash(data));
    }
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
        GIVEN("two cleared registries and a KeyPair") {
            DummyNetworkHandler dnet;
            DummyStorageHandler dstor;
            REL_TIME_PROV_T drelTimeProv(new DummyRelativeTimeProvider);
            BCAST_SOCKET_T bcast = dnet.createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT);

            Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
            Registry<string> reg("someRegistry", &dstor, &dnet, drelTimeProv);
            Registry<string> reg2("someRegistry", &dstor, &dnet, drelTimeProv);

            reg.setBcastSocket(bcast);
            reg2.setBcastSocket(bcast);

            WHEN("Adding an entry to the first registry and executing a force sync") {
                reg.set("test", "blub", pair);
                string entryID = string(reg.entries.back().uuid);
                CAPTURE(entryID);
                reg.sync(true);
//              RHED = DB-A broadcasts its current state

                AND_WHEN("iterating the network exactly four times") {
                    vector<uint8_t> registryData;
//                  FOUR ITERATIONS:
//                      RREQ = DB-B starts a binary search since its head differs from DB-A and in turn requests a hash from DB-A
//                      RHSH = DB-A sends the hash
//                      RREQ = DB-B wants to know if DB-A has corresponding entries to merge as a result of the binary search
//                      RSEN = DB-A has one and sends it
                    for (int i = 0; i <= 4; ++i) {
                        if (bcast->recv(&registryData, 0) == RECV_OK) {
                            const char* identifier = flatbuffers::GetBufferIdentifier(registryData.data());
                            std::string id(identifier, identifier + flatbuffers::FlatBufferBuilder::kFileIdentifierLength);
                            reg.onData(registryData);
                            reg2.onData(registryData);
                            registryData.clear();
                        }
                    }

                    THEN("the second registry should contain the added entry") {
                        REQUIRE(reg2.has("test"));
                    }
                }
            }
        }
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

            // TODO Reimplement these tests according to the new way of doing things!
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
