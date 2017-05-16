#ifndef UCL_REGISTRY_HPP
#define UCL_REGISTRY_HPP

#include <string>
#include <vector>
#include <map>

#include "crypto/crypto.hpp"
#include "json/ArduinoJson.hpp"
using namespace ArduinoJson;
using namespace std;

enum RegistryEntryType {
    INSERT,
    DELETE,
    UPDATE
};

class RegistryEntry {
public:
    // Metadata
    UUID uuid;
    SIGNATURE_T signature;
    PUB_HASH_T publicKeyUsed;
    RegistryEntryType type;

    // Content
    string key;
    string value;

    string getSignatureText() {
        return this->uuid + this->key + this->value + to_string(this->type);
    }

    // Functions
    RegistryEntry(RegistryEntryType type, string key, string value, Crypto::asymmetric::KeyPair pair)
            : type(type), key(key), value(value), uuid(Crypto::generateUUID()), publicKeyUsed(pair.getPublicHash()) {
        this->signature = Crypto::asymmetric::sign(this->getSignatureText(), pair.getPrivate());
    };

    RegistryEntry(string serializedEntry) {
        DynamicJsonBuffer jsonBuffer(600);
        JsonObject& root = jsonBuffer.parseObject(serializedEntry);
        // UUID
        this->uuid = root["metadata"]["uuid"].as<UUID>();
        // Signature
        string signature(root["metadata"]["signature"].as<string>());
        vector<uint8_t> signatureBytes(Crypto::deserializeHexString(signature));
        copy_n(begin(signatureBytes), SIGNATURE_SIZE, begin(this->signature));
        // Public key
        string pubKeyHash = root["metadata"]["publicKeyUsed"].as<string>();
        copy_n(begin(pubKeyHash), PUB_HASH_SIZE, begin(this->publicKeyUsed));
        // Type
        string type = root["metadata"]["type"].as<string>();
        if      (type == "INSERT") this->type = RegistryEntryType::INSERT;
        else if (type == "DELETE") this->type = RegistryEntryType::DELETE;
        else if (type == "UPDATE") this->type = RegistryEntryType::UPDATE;

        // Contents
        this->key = root["content"]["key"].as<string>();
        this->value = root["content"]["value"].as<string>();
    }

    string serialize();

    enum Verify {
        OK,
        PubKeyNotFound,
        SignatureInvalid
    };

    Verify verifySignature(map<PUB_HASH_T, PUBLIC_KEY_T> keys);
};

class Registry {
    vector<RegistryEntry> entries;
    map<string, string> head;
public:
    Registry() {

    }

    void add(string key, string value) {

    }

    void updateHead() {
        head.clear();
        for (const auto &entry : entries) {

        }
    }
};


#endif //UCL_REGISTRY_HPP
