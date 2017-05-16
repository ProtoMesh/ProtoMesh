#ifndef UCL_REGISTRY_HPP
#define UCL_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>

#include "../crypto/crypto.hpp"
#include "../json/ArduinoJson.hpp"
using namespace ArduinoJson;
using namespace std;

enum RegistryEntryType {
    INSERT,
    DELETE,
    UPDATE
};

class RegistryEntry {
    string getSignatureText() { return this->uuid + this->key + this->value + to_string(this->type); }

    // Metadata
    UUID uuid;
    SIGNATURE_T signature;
    PUB_HASH_T publicKeyUsed;
    RegistryEntryType type;

    // Content
    string key;
    string value;

public:
    // Functions
    RegistryEntry(RegistryEntryType type, string key, string value, Crypto::asymmetric::KeyPair pair);

    RegistryEntry(string serializedEntry);

    enum Verify {
        OK,
        PubKeyNotFound,
        SignatureInvalid
    };

    Verify verifySignature(map<PUB_HASH_T, PUBLIC_KEY_T> keys);

    string serialize();
};


#endif //UCL_REGISTRY_HPP
