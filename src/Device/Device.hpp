#ifndef LUMOS_DEVICE_HPP
#define LUMOS_DEVICE_HPP

#include "../api/network.hpp"
#include "../api/storage.hpp"
#include "../crypto/crypto.hpp"
#include "../const.hpp"
#include "../Registry/Registry.hpp"
#include "../api/time.hpp"

class Device {
    NetworkProvider* net;
    StorageProvider* stor;

    BCAST_SOCKET_T deviceBcast;
    BCAST_SOCKET_T registryBcast;

    REL_TIME_PROV_T relTimeProvider;
public:
    vector<Registry<vector<uint8_t>>> registries;

    inline Device(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider) : net(net), stor(stor),
                                            deviceBcast(net->createBroadcastSocket(MULTICAST_NETWORK, DEVICE_PORT)),
                                            registryBcast(net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)),
                                            relTimeProvider(relTimeProvider) {};
    void tick(unsigned int timeoutMS);
};


#endif //LUMOS_DEVICE_HPP
