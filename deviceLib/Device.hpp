#ifndef UCL_DEVICE_HPP
#define UCL_DEVICE_HPP

#include "api/network.hpp"
#include "api/storage.hpp"
#include "crypto/crypto.hpp"

class Device {
    MulticastHandler* mcast;
    UnicastHandler* ucast;
    StorageHandler* stor;
public:
    inline Device(UnicastHandler* ucast, MulticastHandler* mcast, StorageHandler* stor) : mcast(mcast), ucast(ucast), stor(stor) {
        Crypto::asym::verifyKeySize();
        mcast->setTarget("224.17.10.20", 1337);
    };
    void tick(unsigned int timeoutMS);
};


#endif //UCL_DEVICE_HPP
