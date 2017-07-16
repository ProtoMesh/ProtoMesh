#ifndef LUMOS_REGISTRY_ENTRY_HPP
#define LUMOS_REGISTRY_ENTRY_HPP

#include <string>
#include <vector>
#include <map>
#include <result.h>

#include "crypto/crypto.hpp"

#include "flatbuffers/flatbuffers.h"
#include "buffers/uuid_generated.h"
#include "buffers/crypto_generated.h"
#include "buffers/registry/entry_generated.h"

using namespace std;

enum class RegistryEntryType {
    UPSERT,
    DELETE
};

struct RegistryEntryVerificationError {
    enum class Kind { PubKeyNotFound, SignatureInvalid };
    Kind kind;
    std::string text;
    RegistryEntryVerificationError(Kind kind, std::string text) : kind(kind), text(text) {}
};

struct RegistryEntryDeserializationError {
    enum class Kind { WrongType, InvalidData, SignatureSizeMismatch, UsedKeySizeMismatch };
    Kind kind;
    std::string text;
    RegistryEntryDeserializationError(Kind kind, std::string text) : kind(kind), text(text) {}
};

template <class VALUE_T>
class RegistryEntry {
    static Result<RegistryEntry<VALUE_T>, RegistryEntryDeserializationError> loadFromBuffer(const lumos::registry::Entry *entry);
public:
    // Metadata
    Crypto::UUID uuid;
    Crypto::UUID parentUUID;

    SIGNATURE_T signature;
    PUB_HASH_T publicKeyUsed;

    RegistryEntryType type;

    // Content
    string key;
    VALUE_T value;

    // Functions
    RegistryEntry(RegistryEntryType type, string key, VALUE_T value, Crypto::asym::KeyPair pair, Crypto::UUID parentID);
    RegistryEntry(Crypto::UUID uuid, Crypto::UUID parentUUID, SIGNATURE_T signature, PUB_HASH_T publicKeyUsed, RegistryEntryType type, string key, VALUE_T value);

    static Result<RegistryEntry<VALUE_T>, RegistryEntryDeserializationError> fromBuffer(const lumos::registry::Entry* serializedEntry);
    static Result<RegistryEntry<VALUE_T>, RegistryEntryDeserializationError> fromSerialized(vector<uint8_t> serializedEntry);

    vector<uint8_t> getSignatureContent() const;

    Result<bool, RegistryEntryVerificationError> verifySignature(map<PUB_HASH_T, Crypto::asym::PublicKey>* keys) const;

    vector<uint8_t> serialize() const;

    inline bool operator==(const RegistryEntry &other) { return this->serialize() == other.serialize(); }

    flatbuffers::Offset<lumos::registry::Entry> toFlatbufferOffset(flatbuffers::FlatBufferBuilder &builder) const;
};


#endif //LUMOS_REGISTRY_ENTRY_HPP
