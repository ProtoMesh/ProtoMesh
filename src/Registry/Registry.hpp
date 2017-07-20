#ifndef LUMOS_REGISTRY_HPP
#define LUMOS_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include <random>
#include <list>
#include "duktape.h"
#include "Registry/Entry/RegistryEntry.hpp"
#include "Registry/Validators.hpp"
#include "../const.hpp"
#include "../crypto/crypto.hpp"
#include "../api/api.hpp"
#include "../api/storage.hpp"
#include "../api/network.hpp"
#include "../Logger/Logger.hpp"
#include "../api/time.hpp"

#include "../buffers/registry/registry_generated.h"
#include "../buffers/registry/sync/head_generated.h"
#include "../buffers/registry/sync/request_generated.h"
#include "../buffers/registry/sync/entries_generated.h"
#include "../buffers/registry/sync/hash_generated.h"

#define REGISTRY_STORAGE_PREFIX "registry::"
#define LISTENER_T std::function<void(RegistryEntry<vector<uint8_t>>)>

using namespace std;

struct RegistryModificationError {
    enum class Kind { AlreadyPresent, PermissionDenied, SignatureVerificationFailed, ParsingError };
    Kind kind;
    std::string text;
    RegistryModificationError(Kind kind, std::string text) : kind(kind), text(text) {}
};

struct RegistryGetError {
    enum class Kind { EntryNotPresent };
    Kind kind;
    std::string text;
    RegistryGetError(Kind kind, std::string text) : kind(kind), text(text) {}
};

template <class VALUE_T>
class Registry {
#ifdef UNIT_TESTING
public: // Make everything public when unit testing to make the developers life easier and improve readability
#endif
    /// API providers
    APIProvider api;
    BCAST_SOCKET_T bcast;

    /// Instance identification
    string name;
    Crypto::UUID instanceIdentifier;
    map<string, VALUE_T> headState;

    /// Addition of entries
    string validator;
    vector<bool> validateEntries(string validator);
    Result<bool, RegistryModificationError> updateHead(bool save, size_t resultIndex = 0);
    Result<bool, RegistryModificationError> addEntry(RegistryEntry<VALUE_T> newEntry, bool save = true);
    void addEntries(list<RegistryEntry<VALUE_T>> newEntries, size_t startingIndex, bool save = true);
    Result<bool, RegistryModificationError> addSerializedEntry(const lumos::registry::Entry* serialized, bool save = true);
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

    /// Listeners
    vector<LISTENER_T> listeners;
public:
    vector<RegistryEntry<VALUE_T>> entries;
    vector<vector<uint8_t>> hashChain;

    /// Constructor
    Registry(APIProvider api, string name, string validator = DEFAULT_VALIDATOR);

    /// High level data manipulation
    // TODO Add function to check whether or not an insertion would be valid without executing it (useful for e.g. group modification)
    Result<VALUE_T, RegistryGetError> get(string key);
    Result<bool, RegistryModificationError> set(string key, VALUE_T value, Crypto::asym::KeyPair pair);
    Result<bool, RegistryModificationError> del(string key, Crypto::asym::KeyPair pair);
    void clear();

    /// Listeners
    void onChange(LISTENER_T listener);

    /// Synchronization
    void sync(bool force = false);
    void onData(vector<uint8_t> incomingData);

    /// Comparison
    HASH getHeadHash() const;
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


#endif //LUMOS_REGISTRY_HPP
