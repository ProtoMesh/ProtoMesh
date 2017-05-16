#ifndef UCL_DEVICE_HPP
#define UCL_DEVICE_HPP

#include "network.h"
#include "crypto/crypto.hpp"

class Device {
    MulticastHandler* mcast;
    UnicastHandler* ucast;
public:
    Device(UnicastHandler* ucast, MulticastHandler* mcast) : mcast(mcast), ucast(ucast) {
        Crypto::asymmetric::verifyKeySize();
        mcast->setTarget("224.17.10.20", 1337);
    };
    void tick(unsigned int timeoutMS);
};


#endif //UCL_DEVICE_HPP
