#ifndef PROTOMESH_PROTOMESH_HPP
#define PROTOMESH_PROTOMESH_HPP

#include <utility>
#include <memory>

#include <uuid.hpp>
#include <asymmetric.hpp>
#include <TransmissionHandler.hpp>
#include <RelativeTimeProvider.hpp>
#include <Network.hpp>

#include "Device.hpp"
#include "delegates/DeviceHandlerDelegate.hpp"

using namespace std;

namespace ProtoMesh {

    class MeshHandler {
        TRANSMISSION_HANDLER_T transmissionHandler;
        REL_TIME_PROV_T timeProvider;

        communication::Network network;

    public:

        /// Constructors
        explicit MeshHandler(cryptography::UUID deviceID, cryptography::asymmetric::KeyPair deviceKeys, TRANSMISSION_HANDLER_T transmissionHandler, REL_TIME_PROV_T timeProvider);
        static MeshHandler generateNew(TRANSMISSION_HANDLER_T transmissionHandler, REL_TIME_PROV_T timeProvider);

        /// Loop functions
        void tick(unsigned int timeout) {
            vector<uint8_t> buffer;
            this->transmissionHandler->recv(&buffer, timeout);
            if (!buffer.empty()) {
                this->network.processDatagram(buffer);
            }
        }

        /// Delegates
        DEVICE_HANDLER_DELEGATE_T deviceDelegate = nullptr;
    };

}



#endif //PROTOMESH_PROTOMESH_HPP
