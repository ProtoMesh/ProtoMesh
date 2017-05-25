#include "RegistryEntry.hpp"

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

string RegistryEntry::serialize() {
    DynamicJsonBuffer jsonBuffer(600);
    JsonObject& root = jsonBuffer.createObject();

    JsonObject& meta = jsonBuffer.createObject();
    meta["uuid"] = this->uuid;
    meta["parentUUID"] = this->parentUUID;
    meta["signature"] = Crypto::serialize::uint8ArrToString(&this->signature[0], SIGNATURE_SIZE);

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

string RegistryEntry::getSignatureText() {
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

RegistryEntry::Verify RegistryEntry::verifySignature(map<PUB_HASH_T, Crypto::asym::PublicKey*> keys)  {
    // Search for the correct key to use
    auto it = keys.find(this->publicKeyUsed);
    if (it == keys.end()) return Verify::PubKeyNotFound;

    // Verify the signature
    bool res = Crypto::asym::verify(this->getSignatureText(), this->signature, it->second);
    return res ? Verify::OK : Verify::SignatureInvalid;
}