#ifndef PROTOMESH_ROUTING_HPP
#define PROTOMESH_ROUTING_HPP

#include <utility>

#include "transmission.hpp"
#include "uuid.hpp"
#include "asymmetric.hpp"

#include "flatbuffers/flatbuffers.h"
#include "communication/iarp/advertisement_generated.h"

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
        public:
            cryptography::UUID uuid;
            cryptography::asymmetric::PublicKey pubKey;

            vector<cryptography::UUID> route;
        public:
            enum class AdvertisementDeserializationError {
                INVALID_IDENTIFIER,
                INVALID_BUFFER,
                INVALID_PUB_KEY
            };

            explicit Advertisement(cryptography::UUID uuid,
                                   cryptography::asymmetric::PublicKey pubKey,
                                   vector<cryptography::UUID> route = {})
                    : uuid(uuid), pubKey(pubKey), route(std::move(route)) {};

            void addHop(cryptography::UUID uuid);

            vector<uint8_t> serialize() const;

            static Advertisement build(cryptography::UUID uuid, cryptography::asymmetric::KeyPair key) {
                return Advertisement(uuid, key.pub);
            }

            static Result<Advertisement, AdvertisementDeserializationError> fromBuffer(vector<uint8_t> buffer);;
        };
    }

}

#endif // PROTOMESH_ROUTING_HPP