#include "RegistryEntry.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

template <typename VALUE_T>
RegistryEntry<VALUE_T>::RegistryEntry(RegistryEntryType type, string key, VALUE_T value, Crypto::asym::KeyPair pair,
                             Crypto::UUID parentUUID)
        : parentUUID(parentUUID), uuid(Crypto::UUID()), publicKeyUsed(pair.pub.getHash()), type(type), valid(true) , key(key), value(value){
    this->signature = Crypto::asym::sign(this->getSignatureText(), pair.priv);
}

template <typename VALUE_T>
void RegistryEntry<VALUE_T>::loadFromBuffer(const openHome::registry::Entry* entry) {
    using namespace openHome::registry;

    auto uuid = entry->uuid();
    this->uuid = Crypto::UUID(uuid->a(), uuid->b(), uuid->c(), uuid->d());

    auto puuid = entry->parent();
    this->parentUUID = Crypto::UUID(puuid->a(), puuid->b(), puuid->c(), puuid->d());

    switch(entry->type()) {
        case EntryType_UPSERT:
            this->type = RegistryEntryType::UPSERT;
            break;
        case EntryType_DELETE:
            this->type = RegistryEntryType::DELETE;
            break;
    }

    this->key = entry->key()->c_str();
    this->value = entry->value()->c_str(); // TODO Replace w/ uint8_t vector

    auto entry_sig = entry->signature();

    for (flatbuffers::uoffset_t i = 0; i < entry_sig->data()->Length(); i++)
        this->signature[i] = entry_sig->data()->Get(i);

    for (flatbuffers::uoffset_t i = 0; i < entry_sig->used_key()->Length(); i++)
        this->publicKeyUsed[i] = entry_sig->used_key()->Get(i);
}

template <typename VALUE_T>
RegistryEntry<VALUE_T>::RegistryEntry(const openHome::registry::Entry* entry) : valid(true) {
    this->loadFromBuffer(entry);
}

template <typename VALUE_T>
RegistryEntry<VALUE_T>::RegistryEntry(vector<uint8_t> serializedEntry) : valid(true) {
    using namespace openHome::registry;

    // Verify buffer integrity
    auto verifier = flatbuffers::Verifier(serializedEntry.data(), serializedEntry.size());
    if (!VerifyEntryBuffer(verifier)) {
        cerr << "INVALID BUFFER" << endl;
        valid = false;
        return;
    }

    // Check the buffer identifier
    if (!flatbuffers::BufferHasIdentifier(serializedEntry.data(), openHome::registry::EntryIdentifier())) {
        cerr << "Invalid buffer type!" << endl;
        valid = false;
        return;
    }

    auto entry = GetEntry(serializedEntry.data());
    this->loadFromBuffer(entry);
}

template<typename VALUE_T>
flatbuffers::Offset<openHome::registry::Entry> RegistryEntry<VALUE_T>::to_flatbuffer_offset(
        flatbuffers::FlatBufferBuilder &builder) const {
    using namespace openHome::registry;

    openHome::UUID id(this->uuid.a, this->uuid.b, this->uuid.c, this->uuid.d);
    openHome::UUID pid(this->parentUUID.a, this->parentUUID.b, this->parentUUID.c, this->parentUUID.d);

    auto key = builder.CreateString(this->key);
    auto value = builder.CreateString(this->value);

    vector<uint8_t> signature(&this->signature[0], &this->signature[this->signature.size()]);
    vector<uint8_t> usedKey(&this->publicKeyUsed[0], &this->publicKeyUsed[this->publicKeyUsed.size()]);
    auto sig_vec = builder.CreateVector(signature);
    auto pku_vec = builder.CreateVector(usedKey);
    auto sig = openHome::crypto::CreateSignature(builder, sig_vec, pku_vec);

    auto type = EntryType_UPSERT;
    switch (this->type) {
        case RegistryEntryType::UPSERT: type = EntryType_UPSERT; break;
        case RegistryEntryType::DELETE: type = EntryType_DELETE; break;
    }

    return CreateEntry(builder, &id, &pid, type, key, value, sig);
}

template <typename VALUE_T>
vector<uint8_t> RegistryEntry<VALUE_T>::serialize() const {
    using namespace openHome::registry;
    flatbuffers::FlatBufferBuilder builder;

    auto entry = this->to_flatbuffer_offset(builder);
    builder.Finish(entry, EntryIdentifier());

    uint8_t *buf = builder.GetBufferPointer();
    vector<uint8_t> entry_vec(buf, buf + builder.GetSize());

    return entry_vec;
}

template <typename VALUE_T>
string RegistryEntry<VALUE_T>::getSignatureText() const {
    vector<uint8_t> serializedEntry(this->serialize());
    return string(serializedEntry.begin(), serializedEntry.end());
}

template <typename VALUE_T>
SignatureVerificationResult RegistryEntry<VALUE_T>::verifySignature(map<PUB_HASH_T, Crypto::asym::PublicKey *> keys) const {
    // Search for the correct key to use
    auto it = keys.find(this->publicKeyUsed);
    if (it == keys.end()) return SignatureVerificationResult::PubKeyNotFound;

    // Verify the signature
    bool res = Crypto::asym::verify(this->getSignatureText(), this->signature, it->second);
    return res ? SignatureVerificationResult::OK : SignatureVerificationResult::SignatureInvalid;
}

template class RegistryEntry<string>;
//template class RegistryEntry<uint8_t>;

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
        string val("someValue");
        Crypto::UUID parentUUID(0, 1, 2, 3);
        RegistryEntry<string> entry(RegistryEntryType::UPSERT, key, val, pair, parentUUID);

        WHEN("it is serialized") {
            vector<uint8_t> serialized(entry.serialize());

            AND_WHEN("it is reconstructed") {
                RegistryEntry<string> reconstructedEntry(serialized);

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
                    REQUIRE(entry.getSignatureText() == reconstructedEntry.getSignatureText());
                }
            }
        }
    }
}

#endif
