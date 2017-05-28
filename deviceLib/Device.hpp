#ifndef UCL_DEVICE_HPP
#define UCL_DEVICE_HPP

#include "api/network.hpp"
#include "api/storage.hpp"
#include "crypto/crypto.hpp"
#include "const.hpp"
#include "registry/Registry.hpp"
#include "api/time.hpp"

class Device {
    NetworkHandler* net;
    StorageHandler* stor;

    BCAST_SOCKET_T deviceBcast;
    BCAST_SOCKET_T registryBcast;

    REL_TIME_PROV_T relTimeProvider;
public:
    vector<Registry> registries;

    inline Device(NetworkHandler *net, StorageHandler *stor, REL_TIME_PROV_T relTimeProvider) : net(net), stor(stor),
                                                                                                deviceBcast(
                                                                                                        net->createBroadcastSocket(
                                                                                                                MULTICAST_NETWORK,
                                                                                                                DEVICE_PORT)),
                                                                                                registryBcast(
                                                                                                        net->createBroadcastSocket(
                                                                                                                MULTICAST_NETWORK,
                                                                                                                REGISTRY_PORT)),
                                                                                                relTimeProvider(
                                                                                                        relTimeProvider) {
        this->registryBcast->broadcast("test");
    };
    void tick(unsigned int timeoutMS);
};


#endif //UCL_DEVICE_HPP
