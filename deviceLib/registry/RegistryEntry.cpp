#include "RegistryEntry.hpp"

RegistryEntry::RegistryEntry(RegistryEntryType type, string key, string value, Crypto::asymmetric::KeyPair pair)
        : type(type), key(key), value(value), uuid(Crypto::generateUUID()), publicKeyUsed(pair.getPublicHash()) {
    this->signature = Crypto::asymmetric::sign(this->getSignatureText(), pair.getPrivate());
};

RegistryEntry::RegistryEntry(string serializedEntry) {
    DynamicJsonBuffer jsonBuffer(600);
    JsonObject& root = jsonBuffer.parseObject(serializedEntry);
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
    meta["signature"] = Crypto::serialize::uint8ArrToString(&this->signature[0], SIGNATURE_SIZE);

    char keyUsed[PUB_HASH_SIZE+1];
    for (int i = 0; i < PUB_HASH_SIZE; ++i) {
        keyUsed[i] = this->publicKeyUsed[i];
    }
    keyUsed[PUB_HASH_SIZE] = '\0';
    meta["publicKeyUsed"] = keyUsed;

    string type(1, this->type);
    meta["type"] = type;
    switch (this->type) {
        case RegistryEntryType::UPSERT: meta["type"] = "UPSERT"; break;
        case RegistryEntryType::DELETE: meta["type"] = "DELETE"; break;
    }

    JsonObject& content = jsonBuffer.createObject();
    content["key"] = this->key;
    content["value"] = this->value;

    root["metadata"] = meta;
    root["content"] = content;

    string serialized;
    root.prettyPrintTo(serialized);
    return serialized;
}

RegistryEntry::Verify RegistryEntry::verifySignature(map<PUB_HASH_T, PUBLIC_KEY_T> keys)  {
    // Search for the correct key to use
    auto it = keys.find(this->publicKeyUsed);
    if (it == keys.end()) return Verify::PubKeyNotFound;

    // Verify the signature
    bool res = Crypto::asymmetric::verify(this->getSignatureText(), this->signature, it->second);
    return res ? Verify::OK : Verify::SignatureInvalid;
}

string RegistryEntry::getSignatureText() {
    string type;
    switch (this->type) {
        case UPSERT: type = "UPSERT"; break;
        case DELETE: type = "DELETE"; break;
    }
    string uuid;
    copy(begin(this->uuid), end(this->uuid), begin(uuid));
    string key;
    copy(begin(this->key), end(this->key), begin(key));
    string value;
    copy(begin(this->value), end(this->value), begin(value));
    return uuid + key + value + type;
}
