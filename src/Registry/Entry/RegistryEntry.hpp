#ifndef LUMOS_REGISTRY_ENTRY_HPP
#define LUMOS_REGISTRY_ENTRY_HPP

#include <string>
#include <vector>
#include <map>

#include "crypto/crypto.hpp"

#include "flatbuffers/flatbuffers.h"
#include "buffers/uuid_generated.h"
#include "buffers/crypto_generated.h"
#include "buffers/registry/entry_generated.h"

using namespace std;

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
    void loadFromBuffer(const lumos::registry::Entry *entry);
public:
    // Metadata
    Crypto::UUID parentUUID;
    Crypto::UUID uuid;

    SIGNATURE_T signature;
    PUB_HASH_T publicKeyUsed;

    RegistryEntryType type;
    bool valid;

    // Content
    string key;
    VALUE_T value;

    // Functions
    RegistryEntry(RegistryEntryType type, string key, VALUE_T value, Crypto::asym::KeyPair pair, Crypto::UUID parentID);

    RegistryEntry(vector<uint8_t> serializedEntry);
    RegistryEntry(const lumos::registry::Entry* serializedEntry);

    string getSignatureText() const;

    SignatureVerificationResult verifySignature(map<PUB_HASH_T, Crypto::asym::PublicKey *> keys) const;

    vector<uint8_t> serialize() const;

    inline bool operator==(const RegistryEntry &other) { return this->serialize() == other.serialize(); }

    flatbuffers::Offset<lumos::registry::Entry> toFlatbufferOffset(flatbuffers::FlatBufferBuilder &builder) const;
};


#endif //LUMOS_REGISTRY_ENTRY_HPP
