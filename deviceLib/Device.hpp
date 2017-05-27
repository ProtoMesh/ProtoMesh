#ifndef UCL_DEVICE_HPP
#define UCL_DEVICE_HPP

#include "api/network.hpp"
#include "api/storage.hpp"
#include "crypto/crypto.hpp"
#include "const.hpp"

class Device {
    NetworkHandler* net;
    StorageHandler* stor;
public:
    inline Device(NetworkHandler *net, StorageHandler *stor) : net(net), stor(stor) {
        auto bcastSock(net->createBroadcastSocket(MULTICAST_NETWORK, DEVICE_PORT));
        bcastSock->broadcast("test");
    };
    void tick(unsigned int timeoutMS);
};


#endif //UCL_DEVICE_HPP
