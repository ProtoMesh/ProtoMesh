#ifndef OPEN_HOME_NETWORK_H
#define OPEN_HOME_NETWORK_H

#include "../api/time.hpp"
#include "../api/network.hpp"
#include "../api/storage.hpp"
#include "../registry/Registry.hpp"
#include "../crypto/crypto.hpp"

class Network {
    NetworkProvider* net;
    StorageProvider* stor;
    REL_TIME_PROV_T time;

    map<string, Registry> registries;
public:
    Network(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider) : net(net), stor(stor), time(relTimeProvider) {};
};

class NetworkManager {
    NetworkProvider* net;
    StorageProvider* stor;
    REL_TIME_PROV_T time;
public:
    NetworkManager(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider) : net(net), stor(stor), time(relTimeProvider) {};

    Crypto::asym::KeyPair createNetwork() {
        // TODO Initialize the data in the registries for that network
        return Crypto::asym::generateKeyPair();
    }

    Network joinNetwork(string id, NETWORK_KEY_T key) {
        string serializedKey(Crypto::serialize::uint8ArrToString(&key[0], NETWORK_KEY_SIZE));
        this->stor->set("networkKey::" + id, serializedKey);
        this->stor->set("lastJoinedNetwork", id);

        // TODO Mask this->net with the encryption

        cout << "Joining network: " << serializedKey << endl;

        return Network(this->net, this->stor, this->time);
    }

    bool lastJoinedAvailable() {
        return this->stor->has("lastJoinedNetwork");
    }

    Network joinLastNetwork() {
        string lastNetworkID(this->stor->get("lastJoinedNetwork"));
        string serializedKey(this->stor->get("networkKey::" + lastNetworkID));

        vector<uint8_t> deserializedKey(Crypto::serialize::stringToUint8Array(serializedKey));
        array<uint8_t, 33> lastNetworkKey;
        copy(&deserializedKey[0], &deserializedKey[0] + NETWORK_KEY_SIZE, begin(lastNetworkKey));
        return this->joinNetwork(lastNetworkID, lastNetworkKey);
    }
};


#endif //OPEN_HOME_NETWORK_H
