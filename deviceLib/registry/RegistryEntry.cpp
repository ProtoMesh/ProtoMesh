#include "RegistryEntry.hpp"

#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

RegistryEntry::RegistryEntry(RegistryEntryType type, string key, string value, Crypto::asym::KeyPair pair,
                             string parentUUID)
        : type(type), key(key), value(value), uuid(Crypto::generateUUID()), publicKeyUsed(pair.pub.getHash()),
          parentUUID(parentUUID) {
    this->signature = Crypto::asym::sign(this->getSignatureText(), pair.priv);
};

RegistryEntry::RegistryEntry(string serializedEntry) {
    DynamicJsonBuffer jsonBuffer(600);
    JsonObject& root = jsonBuffer.parseObject(serializedEntry);
    // Original hash
    this->parentUUID = root["metadata"]["parentUUID"].as<string>();
    // UUID
    this->uuid = root["metadata"]["uuid"].as<UUID>();
    // Signature
    string signature(root["metadata"]["signature"].as<string>());
    vector<uint8_t> signatureBytes(Crypto::serialize::stringToUint8Array(signature));
    copy_n(begin(signatureBytes), SIGNATURE_SIZE, begin(this->signature));
    // Public key
    string pubKeyHash = root["metadata"]["publicKeyUsed"].as<string>();
    copy_n(begin(pubKeyHash), PUB_HASH_SIZE, begin(this->publicKeyUsed));
    // Type
    string type = root["metadata"]["type"].as<string>();
    if      (type == "UPSERT") this->type = RegistryEntryType::UPSERT;
    else if (type == "DELETE") this->type = RegistryEntryType::DELETE;

    // Contents
    this->key = root["content"]["key"].as<string>();
    this->value = root["content"]["value"].as<string>();
}

RegistryEntry::operator string() const {
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
    return serialized;
}

string RegistryEntry::getSignatureText() const {
    string type;
    switch (this->type) {
        case UPSERT:
            type = "UPSERT";
            break;
        case DELETE:
            type = "DELETE";
            break;
    }
    return this->uuid + this->key + this->value + type;
}

RegistryEntry::Verify RegistryEntry::verifySignature(map<PUB_HASH_T, Crypto::asym::PublicKey *> keys) const {
    // Search for the correct key to use
    auto it = keys.find(this->publicKeyUsed);
    if (it == keys.end()) return Verify::PubKeyNotFound;

    // Verify the signature
    bool res = Crypto::asym::verify(this->getSignatureText(), this->signature, it->second);
    return res ? Verify::OK : Verify::SignatureInvalid;
}

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
        RegistryEntry entry(RegistryEntryType::UPSERT, key, val, pair, parentUUID);

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
                RegistryEntry reconstructedEntry(serialized);

                THEN("it should match the original entry") {
                    REQUIRE(entry == reconstructedEntry);
                }
            }
        }
    }
}

#endif