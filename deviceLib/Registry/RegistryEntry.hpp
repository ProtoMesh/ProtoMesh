#ifndef OPEN_HOME_REGISTRY_ENTRY_HPP
#define OPEN_HOME_REGISTRY_ENTRY_HPP

#include <string>
#include <vector>
#include <map>

#include "../crypto/crypto.hpp"
#include "../json/ArduinoJson.hpp"
using namespace ArduinoJson;
using namespace std;

enum RegistryEntryType {
    INSERT,
    UPDATE,
    UPSERT,
    DELETE
};

class RegistryEntry {
public:
    // Metadata
    string parentUUID;
    UUID uuid;
    SIGNATURE_T signature;
    PUB_HASH_T publicKeyUsed;
    RegistryEntryType type;
    bool valid;

    // Content
    string key;
    string value;

    // Functions
    RegistryEntry(RegistryEntryType type, string key, string value, Crypto::asym::KeyPair pair, string parentHash = "");

    RegistryEntry(string serializedEntry);

    enum Verify {
        OK,
        PubKeyNotFound,
        SignatureInvalid
    };

    string getSignatureText() const;

    Verify verifySignature(map<PUB_HASH_T, Crypto::asym::PublicKey *> keys) const;

    operator string() const;

    inline bool operator==(const RegistryEntry &other) { return string(*this) == string(other); }
};


#endif //OPEN_HOME_REGISTRY_ENTRY_HPP
