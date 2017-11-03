#ifndef PROTOMESH_ROUTING_HPP
#define PROTOMESH_ROUTING_HPP

#include "transmission.hpp"

using namespace std;
using namespace ProtoMesh::Communication;

namespace ProtoMesh::Communication::Routing {

    namespace IARP {
        class RoutingTree {
        public:
            RoutingTree() = default;

            void processAdvertisement(vector<uint8_t> adv) {};
        };

        class Advertisement {
        public:
            Advertisement() {};

            vector<uint8_t> serialize() {
                // TODO Implement
                return {};
            }
        };
    }

}

#endif // PROTOMESH_ROUTING_HPP