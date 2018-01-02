#ifndef PROTOMESH_PROTOMESH_HPP
#define PROTOMESH_PROTOMESH_HPP

#include <utility>

#include <uuid.hpp>
#include <asymmetric.hpp>
#include <TransmissionHandler.hpp>
#include <RelativeTimeProvider.hpp>

namespace ProtoMesh {

    class MeshHandler {
        TRANSMISSION_HANDLER_T transmissionHandler;
        REL_TIME_PROV_T timeProvider;

        cryptography::UUID deviceID;
        cryptography::asymmetric::KeyPair deviceKeys;

    public:
        explicit MeshHandler(cryptography::UUID deviceID, cryptography::asymmetric::KeyPair deviceKeys, TRANSMISSION_HANDLER_T transmissionHandler, REL_TIME_PROV_T timeProvider)
                : transmissionHandler(std::move(transmissionHandler)), timeProvider(std::move(timeProvider)), deviceID(deviceID), deviceKeys(deviceKeys) {};

        static MeshHandler generateNew(TRANSMISSION_HANDLER_T transmissionHandler, REL_TIME_PROV_T timeProvider) {
            return MeshHandler(cryptography::UUID(),
                               cryptography::asymmetric::generateKeyPair(),
                               std::move(transmissionHandler),
                               std::move(timeProvider));
        }
    };

}



#endif //PROTOMESH_PROTOMESH_HPP
