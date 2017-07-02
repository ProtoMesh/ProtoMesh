#include "RegistryEntry.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

template <typename VALUE_T>
RegistryEntry<VALUE_T>::RegistryEntry(RegistryEntryType type, string key, VALUE_T value, Crypto::asym::KeyPair pair,
                             string parentUUID)
        : parentUUID(parentUUID), uuid(Crypto::generateUUID()), publicKeyUsed(pair.pub.getHash()), type(type), valid(true) , key(key), value(value){
    this->signature = Crypto::asym::sign(this->getSignatureText(), pair.priv);
}

template <typename VALUE_T>
RegistryEntry<VALUE_T>::RegistryEntry(string serializedEntry) : valid(true) {

    DynamicJsonBuffer jsonBuffer(600);
    JsonObject& root = jsonBuffer.parseObject(serializedEntry);

    if (root.success()) {
        if (root.containsKey("metadata")) {
            JsonObject& meta = root["metadata"];

            bool sig = meta.containsKey("signature");
            bool pku = meta.containsKey("publicKeyUsed");
            bool uid = meta.containsKey("uuid");
            bool pid = meta.containsKey("parentUUID");
            bool typ = meta.containsKey("type");

            if (!(sig && pku && uid && pid && typ)) { this->valid = false; return; }
        } else { this->valid = false; return; }

        if (root.containsKey("content")) {
            JsonObject& cont = root["content"];

            bool key = cont.containsKey("key");
            bool val = cont.containsKey("value");

            if (!(key && val)) { this->valid = false; return; }
        } else { this->valid = false; return; }

    } else { this->valid = false; return; }

    // Original hash
    this->parentUUID = root["metadata"]["parentUUID"].as<string>();

    // UUID
    this->uuid = root["metadata"]["uuid"].as<UUID>();

    // Signature
    string signature(root["metadata"]["signature"].as<string>());
    vector<uint8_t> signatureBytes(Crypto::serialize::stringToUint8Array(signature));
    if (signatureBytes.size() != SIGNATURE_SIZE) this->valid = false;
    copy_n(begin(signatureBytes), signatureBytes.size(), begin(this->signature));

    // Public key
    string pubKeyHash = root["metadata"]["publicKeyUsed"].as<string>();
    if (pubKeyHash.size() != PUB_HASH_SIZE) this->valid = false;
    copy_n(begin(pubKeyHash), pubKeyHash.size(), begin(this->publicKeyUsed));

    // Type
    string type = root["metadata"]["type"].as<string>();
    if      (type == "UPSERT") this->type = RegistryEntryType::UPSERT;
    else if (type == "DELETE") this->type = RegistryEntryType::DELETE;
    else                       this->valid = false;

    // Contents
    this->key = root["content"]["key"].as<string>();
    this->value = root["content"]["value"].as<VALUE_T>();
}

template <typename VALUE_T>
RegistryEntry<VALUE_T>::operator string() const {
    flatbuffers::FlatBufferBuilder builder;

    auto key = builder.CreateString(this->key);

    auto value = builder.CreateString(this->value);

    openHome::UUID id = openHome::UUID(1, 2, 3, 4);

    vector<uint8_t> signature(&this->signature[0], &this->signature[this->signature.size() - 1]);
    auto vec = builder.CreateVector(signature);
    auto sig = openHome::crypto::CreateSignature(builder, vec, vec);

    EntryBuilder entry_builder(builder);

    // TODO Use reasonable values!
    entry_builder.add_signature(sig);
    entry_builder.add_uuid(&id);
    entry_builder.add_parent(&id);
    entry_builder.add_value(value);
    entry_builder.add_key(key);
    switch (this->type) {
        case RegistryEntryType::UPSERT: entry_builder.add_type(EntryType_UPSERT); break;
        case RegistryEntryType::DELETE: entry_builder.add_type(EntryType_DELETE); break;
    }

    auto entry = entry_builder.Finish();
    builder.Finish(entry);

    uint8_t *buf = builder.GetBufferPointer();
    int size = builder.GetSize();

    cout << size << endl;

    // LEGACY CODE
    DynamicJsonBuffer jsonBuffer(600);
    JsonObject& root = jsonBuffer.createObject();

    JsonObject& meta = jsonBuffer.createObject();
    meta["uuid"] = this->uuid;
    meta["parentUUID"] = this->parentUUID;
    meta["signature"] = Crypto::serialize::uint8ArrToString((uint8_t *) &this->signature[0], SIGNATURE_SIZE);

    char keyUsed[PUB_HASH_SIZE+1];
    for (int i = 0; i < PUB_HASH_SIZE; ++i) {
        keyUsed[i] = this->publicKeyUsed[i];
    }
    keyUsed[PUB_HASH_SIZE] = '\0';
    meta["publicKeyUsed"] = keyUsed;

    switch (this->type) {
        case RegistryEntryType::UPSERT: meta["type"] = "UPSERT"; break;
        case RegistryEntryType::DELETE: meta["type"] = "DELETE"; break;
    }

    JsonObject& content = jsonBuffer.createObject();
    content["key"] = this->key;
    if (this->type == RegistryEntryType::UPSERT) content["value"] = this->value;

    root["metadata"] = meta;
    root["content"] = content;

    string serialized;
    root.printTo(serialized);

    cout << serialized.size() << endl << endl;

    return serialized;
}

template <>
string RegistryEntry<string>::getSignatureText() const {
    string type;
    switch (this->type) {
        case UPSERT:
            type = "UPSERT";
            break;
        case DELETE:
            type = "DELETE";
            break;
    }
    return this->uuid + this->key + this->value + type; // TODO Not all VALUE_T might implement the + operator
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
        string parentUUID("parenthashofdoom");
        RegistryEntry<string> entry(RegistryEntryType::UPSERT, key, val, pair, parentUUID);

        WHEN("it is serialized") {
            string serialized(entry);

            THEN("it should be valid") {
                string pubHashStr(pubHash.begin(), pubHash.end());

                string valid("{\"metadata\":{\"uuid\":\"" + entry.uuid + "\","
                        "\"parentUUID\":\"" + parentUUID + "\","
                                     "\"signature\":\"" +
                             Crypto::serialize::uint8ArrToString(&entry.signature[0], SIGNATURE_SIZE) + "\","
                                     "\"publicKeyUsed\":\"" + pubHashStr + "\",\"type\":\"UPSERT\"},\"content\":"
                                     "{\"key\":\"" + key + "\",\"value\":\"" + val + "\"}}");

                REQUIRE(serialized == valid);
            }

            AND_WHEN("it is reconstructed") {
                RegistryEntry<string> reconstructedEntry(serialized);

                THEN("it should match the original entry") {
                    REQUIRE(entry == reconstructedEntry);
                }
            }
        }
    }
}

#endif
