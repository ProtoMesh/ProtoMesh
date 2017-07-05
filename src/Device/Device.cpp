#include "Device.hpp"

void Device::tick(unsigned int timeoutMS) {
    // Receive data && let the registries broadcast their stuff
    vector<uint8_t> registryData;
    if (this->registryBcast->recv(&registryData, timeoutMS / 2) == RECV_OK) {
        for (auto &reg : this->registries) {
            reg.onData(registryData);
//            reg.sync();
        }
    } else {
//        for (Registry &reg : this->registries) reg.sync();
    }

    // Update the device data
//    vector<uint8_t> deviceData;
//    if (this->deviceBcast->recv(&deviceData, timeoutMS / 2) == RECV_OK)
//        std::cout << deviceData << std::endl; // TODO Handle deviceData
}
