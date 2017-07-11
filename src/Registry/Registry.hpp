#ifndef OPEN_HOME_REGISTRY_HPP
#define OPEN_HOME_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include <random>
#include <list>
#include "duktape.h"
#include "Registry/Entry/RegistryEntry.hpp"
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
#ifdef UNIT_TESTING
public: // Make everything public when unit testing to make the developers life easier and improve readability
#endif
    /// API providers
    StorageProvider* stor;
    NetworkProvider *net;
    REL_TIME_PROV_T relTimeProvider;
    BCAST_SOCKET_T bcast;

    /// Instance identification
    string name;
    Crypto::UUID instanceIdentifier;
    map<string, VALUE_T> headState;

    /// Addition of entries
    vector<bool> validateEntries(string validator);
    void updateHead(bool save);
    bool addEntry(RegistryEntry<VALUE_T> newEntry, bool save = true);
    void addEntries(list<RegistryEntry<VALUE_T>> newEntries, size_t startingIndex, bool save = true);
    bool addSerializedEntry(const openHome::registry::Entry* serialized, bool save = true);
    Crypto::UUID getHeadUUID(); // Helper for addition (getting the corresponding parent)

    /// Synchronization
    bool isSyncInProgress();
    Crypto::UUID requestHash(size_t index, Crypto::UUID target, Crypto::UUID requestID);
    void onBinarySearchResult(size_t index);
    void broadcastEntries(size_t index);
    struct {
        long lastRequestTimestamp;
        Crypto::UUID requestID;
        size_t min;
        size_t max;
        Crypto::UUID communicationTarget;
    } synchronizationStatus;
    long nextBroadcast;

public:
    vector<RegistryEntry<VALUE_T>> entries;
    vector<string> hashChain;

    /// Constructor
    Registry(string name, StorageProvider *stor, NetworkProvider *net, REL_TIME_PROV_T relTimeProvider);

    /// High level data manipulation
    VALUE_T get(string key);
    void set(string key, VALUE_T value, Crypto::asym::KeyPair pair);
    void del(string key, Crypto::asym::KeyPair pair);
    bool has(string key);
    void clear();

    /// Synchronization
    void sync(bool force = false);
    void onData(vector<uint8_t> incomingData);

    /// Comparison
    string getHeadHash() const;
    inline bool operator==(const Registry &other) {
        return this->getHeadHash() == other.getHeadHash();
    }

    /// Printing
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

    /// Unit testing
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
