#ifndef UCL_REGISTRY_HPP
#define UCL_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>
#include "RegistryEntry.hpp"
#include "../crypto/crypto.hpp"

using namespace std;

class Registry {
    vector<RegistryEntry> entries;
    map<string, string> head;
    map<PUB_HASH_T, PUBLIC_KEY_T> trustedKeys;

    bool headOutdated = false;

    void updateHead();
    void addEntry(RegistryEntry e);
public:
    inline Registry(map<PUB_HASH_T, PUBLIC_KEY_T> keys) : trustedKeys(keys) {}

    std::string get(string key);
    void set(string key, string value, Crypto::asymmetric::KeyPair pair);
    void del(string key, Crypto::asymmetric::KeyPair pair);
    bool has(string key);

    void addSerializedEntry(string serialized);
    void setTrustedKeys(map<PUB_HASH_T, PUBLIC_KEY_T> keys);
};


#endif //UCL_REGISTRY_HPP
