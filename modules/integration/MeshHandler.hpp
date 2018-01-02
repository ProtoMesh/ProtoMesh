#ifndef PROTOMESH_PROTOMESH_HPP
#define PROTOMESH_PROTOMESH_HPP

#include <utility>
#include <RelativeTimeProvider.hpp>

#include "TransmissionHandler.hpp"

namespace ProtoMesh {

    class MeshHandler {
        TRANSMISSION_HANDLER_T transmissionHandler;
        REL_TIME_PROV_T timeProvider;

    public:
        explicit MeshHandler(TRANSMISSION_HANDLER_T transmissionHandler, REL_TIME_PROV_T timeProvider)
                : transmissionHandler(std::move(transmissionHandler)), timeProvider(std::move(timeProvider)) {};
    };

}



#endif //PROTOMESH_PROTOMESH_HPP
