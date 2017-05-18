#ifndef UCL_REGISTRY_HPP
#define UCL_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include "RegistryEntry.hpp"
#include "../crypto/crypto.hpp"
#include "../api/storage.hpp"

using namespace std;

class Registry {
    StorageHandler* stor;
    string name;

    vector<RegistryEntry> entries;
    map<string, string> head;
    map<PUB_HASH_T, Crypto::asym::PublicKey*>* trustedKeys;

    void updateHead(bool save);
    void addEntry(RegistryEntry e, bool save = true);
    void storeRegistry();

public:
    Registry(string name, map<PUB_HASH_T, Crypto::asym::PublicKey*>* keys, StorageHandler* stor);

    std::string get(string key);
    void set(string key, string value, Crypto::asym::KeyPair pair);
    void del(string key, Crypto::asym::KeyPair pair);
    bool has(string key);

    void addSerializedEntry(string serialized, bool save = true);
    void setTrustedKeys(map<PUB_HASH_T, Crypto::asym::PublicKey*>* keys);
};


#endif //UCL_REGISTRY_HPP
