#include "Registry.hpp"

#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

template class Registry<vector<uint8_t>>;

/// RNG
random_device rd;
mt19937 rng(rd());
uniform_int_distribution<int> broadcastIntervalRange(REGISTRY_BROADCAST_INTERVAL_MIN, REGISTRY_BROADCAST_INTERVAL_MAX);

/// Helpers
template <typename VALUE_T>
bool Registry<VALUE_T>::isSyncInProgress() {
    return this->api.time->millis() - this->synchronizationStatus.lastRequestTimestamp < REGISTRY_SYNC_TIMEOUT;
}

/// ---------------------------------------------- Synchronization steps ----------------------------------------------
template <typename VALUE_T>
Crypto::UUID Registry<VALUE_T>::requestHash(size_t index, Crypto::UUID target, Crypto::UUID requestID) {
    using namespace lumos::registry::sync;
    flatbuffers::FlatBufferBuilder builder;

    /// Create all request properties
    lumos::UUID rid(requestID.a, requestID.b, requestID.c, requestID.d);
    lumos::UUID tid(target.a, target.b, target.c, target.d);

    /// Create the request
    auto request = CreateRequest(builder, RequestType_HASH, &tid, &rid, index);

    /// Convert it to a byte array
    builder.Finish(request, RequestIdentifier());
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> request_vec(buf, buf + builder.GetSize());

    /// Broadcast the request
    this->bcast->broadcast(request_vec);

    return requestID;
}

template <typename VALUE_T>
void Registry<VALUE_T>::onBinarySearchResult(size_t index) {

    using namespace lumos::registry::sync;
    flatbuffers::FlatBufferBuilder builder;

    /// Create all request properties
    Crypto::UUID target(this->synchronizationStatus.communicationTarget);
    lumos::UUID tid(target.a, target.b, target.c, target.d);

    /// Create the request
    RequestBuilder requestBuilder(builder);

    requestBuilder.add_type(RequestType_ENTRIES);
    requestBuilder.add_target(&tid);
    requestBuilder.add_index(index);

    auto request = requestBuilder.Finish();

    /// Convert it to a byte array
    builder.Finish(request, RequestIdentifier());
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> request_vec(buf, buf + builder.GetSize());

    /// Broadcast the request
    this->bcast->broadcast(request_vec);

    /// Broadcast our entries after the request
    ///     that way the target receives the request first, then sends its new entries out and then merges ours in
    ///     which reduces amount of duplicates on the wire.
    this->broadcastEntries(index);
}

template <typename VALUE_T>
void Registry<VALUE_T>::broadcastEntries(size_t index) { // includes index
    using namespace lumos::registry::sync;

    if (this->entries.size() == 0) return;

    flatbuffers::FlatBufferBuilder builder;

    /// Create the vector of actual registry entries
    vector<flatbuffers::Offset<lumos::registry::Entry>> entryOffsets;
    for (size_t i = index; i < this->entries.size(); i++)
        entryOffsets.push_back(this->entries[i].toFlatbufferOffset(builder));
    auto entries = builder.CreateVector(entryOffsets);

    /// Create all the necessary metadata
    auto name = builder.CreateString(this->name);
    lumos::UUID instance(this->instanceIdentifier.a, this->instanceIdentifier.b, this->instanceIdentifier.c, this->instanceIdentifier.d);

    /// Create the entries
    auto serializedEntries = CreateEntries(builder, name, &instance, entries);

    /// Convert it to a byte array
    builder.Finish(serializedEntries, EntriesIdentifier());
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> entries_vec(buf, buf + builder.GetSize());

    /// Broadcast the head
    this->bcast->broadcast(entries_vec);
}



/// --------------------------------------- Data reception and sync initiation ----------------------------------------

template <typename VALUE_T>
void Registry<VALUE_T>::sync(bool force) {
    /// Check if a sync would be within the defined interval or too early
    if (!force && (this->api.time->millis() < this->nextBroadcast || !this->hashChain.size())) return;
    this->nextBroadcast = this->api.time->millis() + broadcastIntervalRange(rng);

    using namespace lumos::registry::sync;
    flatbuffers::FlatBufferBuilder builder;

    /// Create all properties
    lumos::UUID id(this->instanceIdentifier.a, this->instanceIdentifier.b, this->instanceIdentifier.c, this->instanceIdentifier.d);
    auto name = builder.CreateString(this->name);
    auto headStr = builder.CreateVector(this->getHeadHash());

    /// Create the head buffer
    auto head = CreateHead(builder, name, &id, headStr, this->entries.size());

    /// Convert it to a byte array
    builder.Finish(head, HeadIdentifier());
    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> head_vec(buf, buf + builder.GetSize());

    /// Broadcast the head
    this->bcast->broadcast(head_vec);
}

template <typename VALUE_T>
void Registry<VALUE_T>::onData(vector<uint8_t> incomingData) {
    using namespace lumos::registry::sync;

    /// Generic lambdas and useful variables
    uint8_t* data = incomingData.data();
    auto verifier = flatbuffers::Verifier(data, incomingData.size());
    auto hasIdentifier = [&] (const char * id) -> bool { return flatbuffers::BufferHasIdentifier(data, id); };
    auto onVerifyFail = [] () { cerr << "[REG|SYNC] Received invalid buffer" << endl; };

    /// Lambdas regarding sync stages
    ///   Request related
    auto onEntriesRequest = [&] (const Request* req) {
        if (Crypto::UUID(req->target()) == this->instanceIdentifier)
            this->broadcastEntries(req->index());
    };
    auto onHashRequest = [&] (const Request* req) {
        if (Crypto::UUID(req->target()) == this->instanceIdentifier) {
            flatbuffers::FlatBufferBuilder builder;

            /// Create all hash properties
            auto answerID = lumos::UUID(*req->requestID());
            lumos::UUID instance(this->instanceIdentifier.a, this->instanceIdentifier.b, this->instanceIdentifier.c, this->instanceIdentifier.d);

            vector<uint8_t> rawHashVec;
            if (this->hashChain.size() > req->index()) rawHashVec = this->hashChain[req->index()];
            auto hashVec = builder.CreateVector(rawHashVec);

            /// Create the hash
            auto hash = CreateHash(builder, &answerID, &instance, hashVec);

            /// Convert it to a byte array
            builder.Finish(hash, HashIdentifier());
            uint8_t *buf = builder.GetBufferPointer();
            vector<uint8_t> hash_vec(buf, buf + builder.GetSize());

            /// Broadcast the head
            this->bcast->broadcast(hash_vec);
        }
    };
    ///   Head related
    auto onHead = [&] (const Head* req) {
        vector<uint8_t> head(req->head()->begin(), req->head()->end());
        if (!this->isSyncInProgress() && head != this->getHeadHash()) {
            /// We got a potential candidate for synchronization. Start the binary search!
            size_t l_remote = req->length();
            size_t l_min = min(l_remote, (size_t) this->entries.size()-1);

            Crypto::UUID requestID;
            this->synchronizationStatus.lastRequestTimestamp = this->api.time->millis();
            this->synchronizationStatus.requestID = requestID;
            this->synchronizationStatus.min = 0;
            this->synchronizationStatus.max = l_min;
            this->synchronizationStatus.communicationTarget = Crypto::UUID(req->instance());
            this->requestHash((size_t) ceil(l_min / 2), this->synchronizationStatus.communicationTarget, requestID);
        }
    };
    ///   Entries related
    auto onEntries = [&] (const Entries* entries) {
        if (entries->name()->str() == this->name && Crypto::UUID(entries->instance()) == this->synchronizationStatus.communicationTarget) {
            auto entryOffsets = entries->entries();
            list<RegistryEntry<VALUE_T>> deserializedEntries;
            for (auto offset : *entryOffsets) {
                auto res = RegistryEntry<VALUE_T>::fromBuffer(offset);
                if (res.isOk()) deserializedEntries.push_back(res.unwrap());
            }
            this->addEntries(deserializedEntries, 0); // TODO Replace 0 with the result of the binary search.

            /// Invalidate the current synchronization since it ended with the reception of the entries
            this->synchronizationStatus.lastRequestTimestamp = 0;
        }
    };
    ///   Hash related
    auto onHash = [&] (const Hash* hash) {
        if (Crypto::UUID(hash->answerID()) == this->synchronizationStatus.requestID && this->isSyncInProgress()) {
            size_t min = this->synchronizationStatus.min;
            size_t max = this->synchronizationStatus.max;
            size_t index = (size_t) ceil(min + (max - min) / 2);

            /// The local database does not contain any value beyond the current point (happens when this database is smaller than the other one)
            if (this->hashChain.size() <= index) {
                this->onBinarySearchResult(index);
                return;
            }

            vector<uint8_t> hashVec(hash->hash()->begin(), hash->hash()->end());
            if (this->hashChain[index] == hashVec)
                min = index+1;
            else
                max = index-1;

            if (min > max) {
                this->onBinarySearchResult(min);
            } else if (min == index) {
                this->onBinarySearchResult(index);
            } else {
                Crypto::UUID requestID;
                this->synchronizationStatus.lastRequestTimestamp = this->api.time->millis();
                this->synchronizationStatus.requestID = requestID;
                this->synchronizationStatus.min = min;
                this->synchronizationStatus.max = max;
                this->requestHash((size_t) ceil(min + (max - min) / 2), hash->instance(), requestID);
            }
        }
    };

    /// Actual evaluation of incoming data
    ///   Request ('RREQ')
    if (hasIdentifier(RequestIdentifier())) {
        if (!VerifyRequestBuffer(verifier)) { onVerifyFail(); return; }
        auto request = GetRequest(data);

        // TODO Debounce this. If we processed a similar request a few seconds ago don't answer it again

        switch (request->type()) {
            case RequestType_ENTRIES:
                onEntriesRequest(request);
                break;
            case RequestType_HASH:
                onHashRequest(request);
                break;
        }
    }
    ///   Head state ('RHED')
    else if (hasIdentifier(HeadIdentifier())) {
        if (!VerifyHeadBuffer(verifier)) { onVerifyFail(); return; }
        onHead(GetHead(data));
    }
    ///   Encapsuled registry entry ('RSEN')
    else if (hasIdentifier(EntriesIdentifier())) {
        if (!VerifyEntriesBuffer(verifier)) { onVerifyFail(); return; }
        onEntries(GetEntries(data));
    }
    ///   Entry/chain hash ('RHSH')
    else if (hasIdentifier(HashIdentifier())) {
        if (!VerifyHashBuffer(verifier)) { onVerifyFail(); return; }
        onHash(GetHash(data));
    }
}

#ifdef UNIT_TESTING
    #include "../api/keys.hpp"

    SCENARIO("Database/Registry/Sync", "[registry][sync]") {
            GIVEN("two cleared registries and a KeyPair") {
//            DummyNetworkHandler dnet;
//            DummyStorageHandler dstor;
//            REL_TIME_PROV_T drelTimeProv(new DummyRelativeTimeProvider);
            Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());
            auto key = make_shared<KeyProvider>(pair.pub);
            auto stor = make_shared<DummyStorageHandler>();
            auto net = make_shared<DummyNetworkHandler>();
            auto time = make_shared<DummyRelativeTimeProvider>();
            BCAST_SOCKET_T bcast = net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT);
            APIProvider api = {key, stor, net, time};

            Registry<vector<uint8_t>> reg(api, "someRegistry");
            Registry<vector<uint8_t>> reg2(api, "someRegistry");
            Registry<vector<uint8_t>> reg3(api, "someRegistry");

            reg.setBcastSocket(bcast);
            reg2.setBcastSocket(bcast);
            reg3.setBcastSocket(bcast);
//            reg3.enableDebugging();

            WHEN("Adding an entry to the first registry and executing a force sync") {
                reg.set("test", {1, 2, 3, 4}, pair);
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
                            reg.onData(registryData);
                            reg2.onData(registryData);
                            registryData.clear();
                        }
                    }

                    THEN("the second registry should contain the added entry") {
                        REQUIRE(reg2.has("test"));
                    }

                    THEN("the two registries should be identical") {
                        REQUIRE(reg == reg2);
                    }

                    AND_WHEN("another registry is added, the first two each add an entry and the last one then requests a sync") {
                        reg.set("test2", {1, 2, 3, 4, 5}, pair);
                        reg2.set("test3", {1, 2, 3, 4, 5, 6}, pair);
                        reg2.set("test4", {1}, pair);

                        reg.sync(true);
                        reg2.sync(true);
                        reg3.sync(true);

                        AND_WHEN("the network is iterated a few times") {
                            registryData.clear();
                            for (int i = 0; i <= 100; ++i) {
                                if (bcast->recv(&registryData, 0) == RECV_OK) {
                                    reg.onData(registryData);
                                    reg2.onData(registryData);
                                    reg3.onData(registryData);
                                    registryData.clear();
                                }
                                if (i % 20 == 0) {
                                    reg.sync(true);
                                    reg2.sync(true);
                                    reg3.sync(true);
                                }
                            }

                            THEN("the three registries should be equal") {
                                CAPTURE(reg.getEntries());
                                CAPTURE(reg2.getEntries());
                                CAPTURE(reg3.getEntries());

                                REQUIRE(reg == reg2);
                                REQUIRE(reg2 == reg3);
                            }
                        }
                    }
                }
            }
        }
    }

#endif
