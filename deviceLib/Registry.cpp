#include "Registry.hpp"

string RegistryEntry::serialize() {
    DynamicJsonBuffer jsonBuffer(600);
    JsonObject& root = jsonBuffer.createObject();

    JsonObject& meta = jsonBuffer.createObject();
    meta["uuid"] = this->uuid;
    meta["signature"] = Crypto::serializeIntArray(&this->signature[0], SIGNATURE_SIZE);

    char keyUsed[PUB_HASH_SIZE+1];
    for (int i = 0; i < PUB_HASH_SIZE; ++i) {
        keyUsed[i] = this->publicKeyUsed[i];
    }
    keyUsed[PUB_HASH_SIZE] = '\0';
    meta["publicKeyUsed"] = keyUsed;

    string type(1, this->type);
    meta["type"] = type;
    switch (this->type) {
        case RegistryEntryType::INSERT: meta["type"] = "INSERT"; break;
        case RegistryEntryType::DELETE: meta["type"] = "DELETE"; break;
        case RegistryEntryType::UPDATE: meta["type"] = "UPDATE"; break;
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

RegistryEntry::Verify RegistryEntry::verifySignature(map<PUB_HASH_T, PUBLIC_KEY_T > keys)  {
    // Search for the correct key to use
    auto it = keys.find(this->publicKeyUsed);
    if (it == keys.end()) return Verify::PubKeyNotFound;

    // Verify the signature
    bool res = Crypto::asymmetric::verify(this->getSignatureText(), this->signature, it->second);
    return res ? Verify::OK : Verify::SignatureInvalid;
}