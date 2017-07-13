#include "Network.hpp"

Network::Network(APIProvider api, Crypto::asym::PublicKey masterKey)
    : api(api), masterKey(masterKey), registryBcast(api.net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)) {
    using namespace lumos::network;


    std::function<void(RegistryEntry<vector<uint8_t>>)> listener = [&](RegistryEntry<vector<uint8_t>> entry) {
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
                case UPSERT:
                    this->api.key->insertKey(publicKey.getHash(), publicKey);
                    break;
                case DELETE:
                    this->api.key->removeKey(publicKey.getHash());
                    break;
            }
        }
    };

    this->loadRegistry("network::nodes")->onChange(listener);
}

shared_ptr<Registry<vector<uint8_t>>> Network::loadRegistry(string name) {
    auto sp = std::make_shared<Registry<vector<uint8_t>>>(api, name);

    sp->clear(); cout << "CLEAR" << endl; // TODO Remove this

    this->registries.emplace(name, sp);
    return sp;
}

void Network::tick(unsigned int timeoutMS) {
    // Receive data && let the registries broadcast their stuff
    vector<uint8_t> registryData;
    if (this->registryBcast->recv(&registryData, timeoutMS / 2) == RECV_OK) {
        for (auto reg : this->registries) {
            STORED_REGISTRY_T r = reg.second;
            r->onData(registryData);
            r->sync();
        }
    } else {
        for (auto reg : this->registries) {
            STORED_REGISTRY_T r = reg.second;
            r->sync();
        }
    }
}
