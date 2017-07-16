#include "Network.hpp"

void Network::onNodeRegistryChange(RegistryEntry<vector<uint8_t>> entry) {
    using namespace lumos::network;

    /// Update api.key list according to change in node registry
    auto verifier = flatbuffers::Verifier(entry.value.data(), entry.value.size());
    if (VerifyNodeBuffer(verifier) && flatbuffers::BufferHasIdentifier(entry.value.data(), NodeIdentifier())) {
        auto node = GetNode(entry.value.data());

        /// Load the public key and abort if its size is not correct
        vector<uint8_t> compressedData(node->publicKey()->compressed()->begin(), node->publicKey()->compressed()->end());
        if (compressedData.size() != COMPRESSED_PUB_KEY_SIZE) return;
        array<uint8_t, COMPRESSED_PUB_KEY_SIZE> compressedKey;
        copy_n(compressedData.begin(), COMPRESSED_PUB_KEY_SIZE, compressedKey.begin());
        Crypto::asym::PublicKey publicKey(compressedKey);

        /// Update the key api according to the change type
        switch (entry.type) {
            case RegistryEntryType::UPSERT:
                this->api.key->insertKey(publicKey.getHash(), publicKey);
                break;
            case RegistryEntryType::DELETE:
                this->api.key->removeKey(publicKey.getHash());
                break;
        }
    }
}

void Network::registerNode(Crypto::UUID uid, vector<uint8_t> node, Crypto::asym::KeyPair authorization) {
    cout << "Registered node: " << uid << " using key " << string(authorization.pub.getHash().begin()) << endl;
    this->getRegistry(NODES_REGISTRY)->set(uid, node, authorization);
}