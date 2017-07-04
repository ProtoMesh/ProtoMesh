#ifndef OPEN_HOME_REGISTRY_HPP
#define OPEN_HOME_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include "RegistryEntry.hpp"
#include "../const.hpp"
#include "../crypto/crypto.hpp"
#include "../api/storage.hpp"
#include "../api/network.hpp"
#include "../api/time.hpp"
#include <random>

#include "flatbuffers/flatbuffers.h"
#include "../buffers/registry/registry_generated.h"
#include "../buffers/registry/sync/head_generated.h"
#include "../buffers/registry/sync/request_generated.h"
#include "../buffers/registry/sync/entry_generated.h"
#include "../buffers/registry/sync/hash_generated.h"

#define REGISTRY_STORAGE_PREFIX "registry::"

using namespace std;

template <class VALUE_T>
class Registry {
    // Variables
    StorageProvider* stor;
    NetworkProvider *net;
    REL_TIME_PROV_T relTimeProvider;

    BCAST_SOCKET_T bcast;
    long nextBroadcast;

    string name;
    Crypto::UUID instanceIdentifier;

    map<string, VALUE_T> headState;

    struct {
        long lastRequestTimestamp;
        Crypto::UUID requestID;
        size_t min;
        size_t max;
        Crypto::UUID communicationTarget;
    } synchronizationStatus;

    // Functions
    void updateHead(bool save);
    bool addEntry(RegistryEntry<VALUE_T> e, bool save = true);

    Crypto::UUID getHeadUUID();

    tuple<vector<unsigned long>, unsigned long> getBlockBorders(Crypto::UUID parentUUID = Crypto::UUID::Empty());

    Crypto::UUID requestHash(size_t index, Crypto::UUID target, Crypto::UUID requestID); // requestID = UUID
    void onBinarySearchResult(size_t index);
    void broadcastEntries(size_t index);
    bool isSyncInProgress();

public:
    Registry(string name, StorageProvider *stor, NetworkProvider *net,
             REL_TIME_PROV_T relTimeProvider);

    string get(string key);
    void set(string key, string value, Crypto::asym::KeyPair pair);
    void del(string key, Crypto::asym::KeyPair pair);
    bool has(string key);

    bool addSerializedEntry(const openHome::registry::Entry* serialized, bool save = true);

    vector<string> hashChain;
    string getHeadHash() const;

    void clear();

    void sync();

    void onData(vector<uint8_t> incomingData);

    inline void print() {
        cout << string(this->instanceIdentifier) << " | " << this->entries.size() << endl;
        cout << "No printing....sorry" << endl;
//        for (unsigned int i = 0; i < this->entries.size(); i++) {
//            cout << string(this->entries[i]) << endl;
//        }
//        for (auto &entry : this->entries) cout << string(entry) << endl;
    };
    vector<RegistryEntry<VALUE_T>> entries;
};


#endif //OPEN_HOME_REGISTRY_HPP
