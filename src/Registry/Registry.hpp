#ifndef OPEN_HOME_REGISTRY_HPP
#define OPEN_HOME_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include <random>
#include <list>
#include "RegistryEntry.hpp"
#include "../const.hpp"
#include "../crypto/crypto.hpp"
#include "../api/storage.hpp"
#include "../api/network.hpp"
#include "../api/time.hpp"

#include "flatbuffers/flatbuffers.h"
#include "../buffers/registry/registry_generated.h"
#include "../buffers/registry/sync/head_generated.h"
#include "../buffers/registry/sync/request_generated.h"
#include "../buffers/registry/sync/entries_generated.h"
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
    bool addEntry(RegistryEntry<VALUE_T> newEntry, bool save = true);
    void addEntries(list<RegistryEntry<VALUE_T>> newEntries, size_t startingIndex, bool save = true);

    Crypto::UUID getHeadUUID();

    Crypto::UUID requestHash(size_t index, Crypto::UUID target, Crypto::UUID requestID); // requestID = UUID
    void onBinarySearchResult(size_t index);
    void broadcastEntries(size_t index);
    bool isSyncInProgress();

public:
    vector<RegistryEntry<VALUE_T>> entries;
    vector<string> hashChain;

    Registry(string name, StorageProvider *stor, NetworkProvider *net,
             REL_TIME_PROV_T relTimeProvider);

    VALUE_T get(string key);
    void set(string key, VALUE_T value, Crypto::asym::KeyPair pair);
    void del(string key, Crypto::asym::KeyPair pair);
    bool has(string key);

    bool addSerializedEntry(const openHome::registry::Entry* serialized, bool save = true);

    string getHeadHash() const;

    void clear();

    void sync(bool force = false);

    void onData(vector<uint8_t> incomingData);

    inline bool operator==(const Registry &other) {
        return this->getHeadHash() == other.getHeadHash();
    }

    inline string getEntries() {
        stringstream ss;
        ss << "-------------------------------------------------------------------------------" << endl;
        ss << "Registry: " << this->name << endl;
        ss << "Instance: " << this->instanceIdentifier << endl;
        ss << endl;
        for (auto entr : this->entries) ss << "entry: " << entr.uuid << "; parent: " << entr.parentUUID << endl;
        ss << endl;

        return ss.str();
    }

#ifdef UNIT_TESTING
    bool debug = false;

    inline void enableDebugging() {
        this->debug = true;
    }

    inline void setBcastSocket(BCAST_SOCKET_T sock) {
        this->bcast = sock;
    }
#endif
};


#endif //OPEN_HOME_REGISTRY_HPP
