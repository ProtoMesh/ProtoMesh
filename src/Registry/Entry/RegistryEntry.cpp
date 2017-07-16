#include "RegistryEntry.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

template <typename VALUE_T>
RegistryEntry<VALUE_T>::RegistryEntry(RegistryEntryType type, string key, VALUE_T value, Crypto::asym::KeyPair pair,
                             Crypto::UUID parentUUID)
        : uuid(), parentUUID(parentUUID), publicKeyUsed(pair.pub.getHash()), type(type), key(key), value(value){
    this->signature = Crypto::asym::sign(this->getSignatureContent(), pair.priv);
}

template <typename VALUE_T>
RegistryEntry<VALUE_T>::RegistryEntry(Crypto::UUID uuid, Crypto::UUID parentUUID, SIGNATURE_T signature, PUB_HASH_T publicKeyUsed, RegistryEntryType type, string key, VALUE_T value)
    : uuid(uuid), parentUUID(parentUUID), signature(signature), publicKeyUsed(publicKeyUsed), type(type), key(key), value(value) {}

template <typename VALUE_T>
Result<RegistryEntry<VALUE_T>, DeserializationError> RegistryEntry<VALUE_T>::loadFromBuffer(const lumos::registry::Entry* entry) {
    using namespace lumos::registry;

    /// UUID and its parent
    Crypto::UUID uuid(entry->uuid()->a(), entry->uuid()->b(), entry->uuid()->c(), entry->uuid()->d());
    Crypto::UUID parentUUID(entry->parent()->a(), entry->parent()->b(), entry->parent()->c(), entry->parent()->d());

    /// Type
    RegistryEntryType type = RegistryEntryType::UPSERT;
    switch(entry->type()) {
        case EntryType_UPSERT:
            type = RegistryEntryType::UPSERT;
            break;
        case EntryType_DELETE:
            type = RegistryEntryType::DELETE;
            break;
    }

    /// Key and value
    string key(entry->key()->c_str());
    vector<uint8_t> value(entry->value()->begin(), entry->value()->end());

    /// Signature
    SIGNATURE_T signature;
    PUB_HASH_T publicKeyUsed;
    auto entry_sig = entry->signature();

    /// Check the signature length for mismatches
    if (entry_sig->data()->Length() != SIGNATURE_SIZE)
        return Err(DeserializationError(DeserializationError::Kind::SignatureSizeMismatch, "Signature size of buffer was invalid."));
    if (entry_sig->used_key()->Length() != PUB_HASH_SIZE)
        return Err(DeserializationError(DeserializationError::Kind::UsedKeySizeMismatch, "PKU size of buffer was invalid."));

    for (flatbuffers::uoffset_t i = 0; i < entry_sig->data()->Length(); i++)
        signature[i] = entry_sig->data()->Get(i);

    for (flatbuffers::uoffset_t i = 0; i < entry_sig->used_key()->Length(); i++)
        publicKeyUsed[i] = entry_sig->used_key()->Get(i);


    /// Create and return the entry
    return Ok(RegistryEntry(uuid, parentUUID, signature, publicKeyUsed, type, key, value));
}

template<typename VALUE_T>
Result<RegistryEntry<VALUE_T>, DeserializationError> RegistryEntry<VALUE_T>::fromBuffer(const lumos::registry::Entry* serializedEntry) {
    return RegistryEntry::loadFromBuffer(serializedEntry);
};

template<typename VALUE_T>
Result<RegistryEntry<VALUE_T>, DeserializationError> RegistryEntry<VALUE_T>::fromSerialized(vector<uint8_t> serializedEntry) {
    using namespace lumos::registry;

    /// Verify buffer integrity
    auto verifier = flatbuffers::Verifier(serializedEntry.data(), serializedEntry.size());
    if (!VerifyEntryBuffer(verifier))
        return Err(DeserializationError(DeserializationError::Kind::InvalidData, "Passed data is no valid registry entry!"));

    /// Check the buffer identifier
    if (!flatbuffers::BufferHasIdentifier(serializedEntry.data(), lumos::registry::EntryIdentifier()))
        return Err(DeserializationError(DeserializationError::Kind::WrongType, "Passed data is not a registry entry!"));

    /// Load and return the entry
    auto entry = GetEntry(serializedEntry.data());
    return RegistryEntry::loadFromBuffer(entry);
}

template<typename VALUE_T>
flatbuffers::Offset<lumos::registry::Entry> RegistryEntry<VALUE_T>::toFlatbufferOffset(
        flatbuffers::FlatBufferBuilder &builder) const {
    using namespace lumos::registry;

    lumos::UUID id(this->uuid.a, this->uuid.b, this->uuid.c, this->uuid.d);
    lumos::UUID pid(this->parentUUID.a, this->parentUUID.b, this->parentUUID.c, this->parentUUID.d);

    auto key = builder.CreateString(this->key);
    vector<uint8_t> val(this->value.data(), &this->value[this->value.size()]);
    auto value = builder.CreateVector(val);

    vector<uint8_t> signature(this->signature.data(), &this->signature[this->signature.size()]);
    vector<uint8_t> usedKey(this->publicKeyUsed.data(), &this->publicKeyUsed[this->publicKeyUsed.size()]);
    auto sig_vec = builder.CreateVector(signature);
    auto pku_vec = builder.CreateVector(usedKey);
    auto sig = lumos::crypto::CreateSignature(builder, sig_vec, pku_vec);

    auto type = EntryType_UPSERT;
    switch (this->type) {
        case RegistryEntryType::UPSERT: type = EntryType_UPSERT; break;
        case RegistryEntryType::DELETE: type = EntryType_DELETE; break;
    }

    return CreateEntry(builder, &id, &pid, type, key, value, sig);
}

template <typename VALUE_T>
vector<uint8_t> RegistryEntry<VALUE_T>::serialize() const {
    using namespace lumos::registry;
    flatbuffers::FlatBufferBuilder builder;

    auto entry = this->toFlatbufferOffset(builder);
    builder.Finish(entry, EntryIdentifier());

    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> entry_vec(buf, buf + builder.GetSize());

    return entry_vec;
}

template <typename VALUE_T>
vector<uint8_t> RegistryEntry<VALUE_T>::getSignatureContent() const {
    vector<uint8_t> content(this->value.begin(), this->value.end());

    auto pushAll = [&] (vector<uint8_t> vec) {
        for (uint8_t v : vec) content.push_back(v);
    };

    pushAll(this->uuid.toVector());
    pushAll(this->parentUUID.toVector());

    vector<uint8_t> key(this->key.begin(), this->key.end());
    pushAll(key);

    switch (this->type) {
        case RegistryEntryType::UPSERT: content.push_back(0); break;
        case RegistryEntryType::DELETE: content.push_back(1); break;
    }

    return content;
}

template <typename VALUE_T>
Result<bool, VerificationError> RegistryEntry<VALUE_T>::verifySignature(map<PUB_HASH_T, Crypto::asym::PublicKey>* keys) const {
    // Search for the correct key to use
    auto it = keys->find(this->publicKeyUsed);
    if (it == keys->end()) return Err(VerificationError(VerificationError::Kind::PubKeyNotFound, "Public key not found!"));

    // Verify the signature
    if (Crypto::asym::verify(this->getSignatureContent(), this->signature, &it->second))
        return Ok(true);
    else
        return Err(VerificationError(VerificationError::Kind::SignatureInvalid, "Signature is invalid."));
}

template class RegistryEntry<vector<uint8_t>>;

#ifdef UNIT_TESTING

SCENARIO("RegistryEntries", "[registry][entry]") {
    // Generate a static Keypair
    string serializedPriv("c0fbe1f7ffbfc953c0a8be3cecb9ee77b30503fdc9ec7fafa0a38a81b20a706d");
    string serializedPub(
            "0a7a644db56de11043670fdb50e9e47f02eecc656ecf46436cbe3e0f916950ba5008a6a6146d20f042dff36c43ebaa22bca994866cbb0f7124c039b043ee8bf2");

    vector<uint8_t> privKey(Crypto::serialize::stringToUint8Array(serializedPriv));
    vector<uint8_t> pubKey(Crypto::serialize::stringToUint8Array(serializedPub));

    Crypto::asym::KeyPair pair(&privKey[0], &pubKey[0]);
    PUB_HASH_T pubHash(pair.pub.getHash());

    GIVEN("A basic UPSERT registry entry") {
        string key("someKey");
        vector<uint8_t> val = {1,2,3,4,5};
        Crypto::UUID parentUUID(0, 1, 2, 3);
        RegistryEntry<vector<uint8_t>> entry(RegistryEntryType::UPSERT, key, val, pair, parentUUID);

        WHEN("it is serialized") {
            vector<uint8_t> serialized(entry.serialize());

            AND_WHEN("it is reconstructed") {
                RegistryEntry<vector<uint8_t>> reconstructedEntry(RegistryEntry<vector<uint8_t>>::fromSerialized(serialized).unwrap());

                THEN("it should match the original entry") {
                    CAPTURE(entry.serialize());
                    CAPTURE(reconstructedEntry.serialize());
                    REQUIRE(entry == reconstructedEntry);
                }

                THEN("it's key/value should equal the original one") {
                    REQUIRE(reconstructedEntry.key == key);
                    REQUIRE(reconstructedEntry.value == val);
                }

                THEN("it's signature text should match the original") {
                    REQUIRE(entry.getSignatureContent() == reconstructedEntry.getSignatureContent());
                }
            }
        }
    }
}

#endif
