#include "NetworkManager.hpp"

NETWORK NetworkManager::joinNetwork(string id, NETWORK_KEY_T key)  {
    string serializedKey(Crypto::serialize::uint8ArrToString(key.data(), NETWORK_KEY_SIZE));
    this->api.stor->set("networkKey::" + id, serializedKey);
    this->api.stor->set("lastJoinedNetwork", id);

    // TODO Mask this->net with the encryption

    Crypto::asym::PublicKey masterKey(key);
    Logger(Info) << "Joining network " << string(masterKey.getHash().begin(), PUB_HASH_SIZE) << endl;

    APIProvider api = this->api;
    auto keyProvider = std::make_shared<KeyProvider>(masterKey);
    api.key = keyProvider; // TODO Escapes local scope

    return make_shared<Network>(api, masterKey);
}

bool NetworkManager::lastJoinedAvailable()  {
    return this->api.stor->has("lastJoinedNetwork");
}

NETWORK NetworkManager::joinLastNetwork()  {
    string lastNetworkID(this->api.stor->get_str("lastJoinedNetwork"));
    string serializedKey(this->api.stor->get_str("networkKey::" + lastNetworkID));

    vector<uint8_t> deserializedKey(Crypto::serialize::stringToUint8Array(serializedKey));
    array<uint8_t, 33> lastNetworkKey;
    copy(deserializedKey.data(), deserializedKey.data() + NETWORK_KEY_SIZE, begin(lastNetworkKey));
    return this->joinNetwork(lastNetworkID, lastNetworkKey);
}
