#ifndef PROTOMESH_ROUTING_HPP
#define PROTOMESH_ROUTING_HPP

#include "transmission.hpp"
#include "uuid.hpp"

using namespace std;
using namespace ProtoMesh::communication;

namespace ProtoMesh::communication::Routing {

    namespace IARP {
        class RoutingTable {
        public:
            RoutingTable() = default;

            void processAdvertisement(vector<uint8_t> adv) {};
        };

        class Advertisement {
            cryptography::UUID uuid;
        public:
            explicit Advertisement(cryptography::UUID uuid) : uuid(uuid) {};

            vector<uint8_t> serialize() {
                // TODO Implement
                return {};
            }
        };
    }

}

#endif // PROTOMESH_ROUTING_HPP