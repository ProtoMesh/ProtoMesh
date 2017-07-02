#ifndef OPEN_HOME_DEVICE_HPP
#define OPEN_HOME_DEVICE_HPP

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
    vector<Registry> registries;

    inline Device(NetworkProvider *net, StorageProvider *stor, REL_TIME_PROV_T relTimeProvider) : net(net), stor(stor),
                                            deviceBcast(net->createBroadcastSocket(MULTICAST_NETWORK, DEVICE_PORT)),
                                            registryBcast(net->createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT)),
                                            relTimeProvider(relTimeProvider) {};
    void tick(unsigned int timeoutMS);
    inline void print() {
        this->registries[0].print();
        std::cout << std::endl;
        this->registries[1].print();
    }
};


#endif //OPEN_HOME_DEVICE_HPP
