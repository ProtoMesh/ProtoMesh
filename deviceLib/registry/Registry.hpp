#ifndef UCL_REGISTRY_HPP
#define UCL_REGISTRY_HPP

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

using namespace std;

class Registry {
    StorageHandler* stor;
    NetworkHandler *net;
    REL_TIME_PROV_T relTimeProvider;

    BCAST_SOCKET_T bcast;
    long nextBroadcast;

    string name;

    map<string, string> headState;
    map<PUB_HASH_T, Crypto::asym::PublicKey*>* trustedKeys;

    void updateHead(bool save);
    void addEntry(RegistryEntry e, bool save = true);

    string getHeadUUID();

    tuple<vector<unsigned long>, unsigned long> getBlockBorders(string parentUUID = "");

public:
    Registry(string name, map<PUB_HASH_T, Crypto::asym::PublicKey *> *keys, StorageHandler *stor, NetworkHandler *net,
             REL_TIME_PROV_T relTimeProvider);

    string get(string key);
    void set(string key, string value, Crypto::asym::KeyPair pair);
    void del(string key, Crypto::asym::KeyPair pair);
    bool has(string key);

    void addSerializedEntry(string serialized, bool save = true);
    void setTrustedKeys(map<PUB_HASH_T, Crypto::asym::PublicKey*>* keys);

    vector<string> hashChain;
    string getHeadHash() const;

    void clear();

    void sync();

    void onData(string request);

    inline void print() {
        for (auto &entry : this->entries) cout << string(entry) << endl;
    };
    vector<RegistryEntry> entries;
};


#endif //UCL_REGISTRY_HPP
