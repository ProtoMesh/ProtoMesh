#include "Network.hpp"

void Network::onNodeRegistryChange(RegistryEntry<vector<uint8_t>> entry) {
    using namespace lumos::network;

    /// Update api.key list according to change in node registry
    auto verifier = flatbuffers::Verifier(entry.value.data(), entry.value.size());
    if (VerifyNodeBuffer(verifier) && flatbuffers::BufferHasIdentifier(entry.value.data(), NodeIdentifier())) {
        auto node = GetNode(entry.value.data());

        /// Load the public key and abort if its size is not correct
        auto publicKey = Crypto::asym::PublicKey::fromBuffer(node->publicKey()->compressed());

        if (publicKey.isErr()) return;

        /// Update the key api according to the change type
        switch (entry.type) {
            case RegistryEntryType::UPSERT:
                this->api.key->insertKey(publicKey.unwrap().getHash(), publicKey.unwrap());
                break;
            case RegistryEntryType::DELETE:
                this->api.key->removeKey(publicKey.unwrap().getHash());
                break;
        }
    }
}

void Network::registerNode(Crypto::UUID uid, vector<uint8_t> node, Crypto::asym::KeyPair authorization) {
    this->getRegistry(NODES_REGISTRY)->set(uid, node, authorization);

    // TODO DEBUG OUTPUT
    auto hash = authorization.pub.getHash();
    cout << "Registered node: " << uid << " using key " << string(hash.begin(), hash.end()) << endl;
}