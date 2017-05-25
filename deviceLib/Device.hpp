#ifndef UCL_DEVICE_HPP
#define UCL_DEVICE_HPP

#include "api/network.hpp"
#include "api/storage.hpp"
#include "crypto/crypto.hpp"

class Device {
    NetworkHandler *net;
    StorageHandler* stor;
public:
    inline Device(NetworkHandler *net, StorageHandler *stor) : net(net), stor(stor) {
        Crypto::asym::verifyKeySize();
        net->setBroadcastTarget("224.17.10.20", 1337);
    };
    void tick(unsigned int timeoutMS);
};


#endif //UCL_DEVICE_HPP
