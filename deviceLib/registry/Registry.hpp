#ifndef UCL_REGISTRY_HPP
#define UCL_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include "RegistryEntry.hpp"
#include "../crypto/crypto.hpp"
#include "../api/storage.hpp"
#include "../api/network.hpp"

using namespace std;

class Registry {
    StorageHandler* stor;
    NetworkHandler *net;
    string name;

    vector<string> hashChain;
    map<string, string> head;
    map<PUB_HASH_T, Crypto::asym::PublicKey*>* trustedKeys;

    void updateHead(bool save);
    void addEntry(RegistryEntry e, bool save = true);

    string getHeadUUID();

    tuple<vector<unsigned long>, unsigned long> getBlockBorders(string parentUUID = "");

public:
    Registry(string name, map<PUB_HASH_T, Crypto::asym::PublicKey *> *keys, StorageHandler *stor, NetworkHandler *net);

    std::string get(string key);
    void set(string key, string value, Crypto::asym::KeyPair pair);
    void del(string key, Crypto::asym::KeyPair pair);
    bool has(string key);

    void addSerializedEntry(string serialized, bool save = true);
    void setTrustedKeys(map<PUB_HASH_T, Crypto::asym::PublicKey*>* keys);

    void clear();

    void sync();

    void onSyncRequest(string request);

    inline void print() {
        for (auto &entry : this->entries) {
            std::cout << entry.serialize() << std::endl;
        }
    };
    vector<RegistryEntry> entries;
};


#endif //UCL_REGISTRY_HPP
