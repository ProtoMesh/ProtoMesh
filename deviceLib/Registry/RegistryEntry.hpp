#ifndef OPEN_HOME_REGISTRY_ENTRY_HPP
#define OPEN_HOME_REGISTRY_ENTRY_HPP

#include <string>
#include <vector>
#include <map>

#include "../crypto/crypto.hpp"
#include "../json/ArduinoJson.hpp"

#include "flatbuffers/flatbuffers.h"
#include "../buffers/uuid_generated.h"
#include "../buffers/crypto_generated.h"
#include "../buffers/registry/entry_generated.h"

using namespace ArduinoJson;
using namespace std;
using namespace openHome::registry;

enum RegistryEntryType {
    UPSERT,
    DELETE
};

enum SignatureVerificationResult {
    OK,
    PubKeyNotFound,
    SignatureInvalid
};

template <class VALUE_T>
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
    VALUE_T value;

    // Functions
    RegistryEntry(RegistryEntryType type, string key, VALUE_T value, Crypto::asym::KeyPair pair, string parentHash = "");

    RegistryEntry(string serializedEntry);

    string getSignatureText() const;

    SignatureVerificationResult verifySignature(map<PUB_HASH_T, Crypto::asym::PublicKey *> keys) const;

    operator string() const;

    inline bool operator==(const RegistryEntry &other) { return string(*this) == string(other); }
};


#endif //OPEN_HOME_REGISTRY_ENTRY_HPP
