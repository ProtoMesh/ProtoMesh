#ifndef PROTOMESH_ROUTING_HPP
#define PROTOMESH_ROUTING_HPP

#include <utility>
#include <unordered_map>

#include "transmission.hpp"
#include "uuid.hpp"
#include "asymmetric.hpp"

#include "flatbuffers/flatbuffers.h"
#include "communication/iarp/advertisement_generated.h"

using namespace std;
using namespace ProtoMesh::communication;

namespace ProtoMesh::communication::Routing {

    namespace IARP {

        class Advertisement {
        public:
            cryptography::UUID uuid;
            cryptography::asymmetric::PublicKey pubKey;

            vector<cryptography::UUID> route;
            unsigned int interval;

            enum class AdvertisementDeserializationError {
                INVALID_IDENTIFIER,
                INVALID_BUFFER,
                INVALID_PUB_KEY
            };

            explicit Advertisement(cryptography::UUID uuid,
                                   cryptography::asymmetric::PublicKey pubKey,
                                   vector<cryptography::UUID> route = {},
                                   unsigned int interval = 10000)
                    : uuid(uuid), pubKey(pubKey), route(std::move(route)), interval(interval) {};

            void addHop(cryptography::UUID uuid);

            vector<uint8_t> serialize() const;

            static Advertisement build(cryptography::UUID uuid, cryptography::asymmetric::KeyPair key) {
                return Advertisement(uuid, key.pub);
            }

            static Result<Advertisement, AdvertisementDeserializationError> fromBuffer(vector<uint8_t> buffer);
        };

        class RoutingTableEntry {
        public:
            long validUntil;
            vector<cryptography::UUID> route;

            RoutingTableEntry(Advertisement adv, long currentTime)
                    : validUntil(currentTime + adv.interval), route(adv.route) {
                std::reverse(this->route.begin(), this->route.end());
            };
        };

        enum class RouteDiscoveryError {
            NO_ROUTE_AVAILABLE
        };

        class RoutingTable {
            unordered_map<cryptography::UUID, vector<RoutingTableEntry>> routes;

        public:
            Result<vector<cryptography::UUID>, RouteDiscoveryError> getRouteTo(cryptography::UUID uuid);

            void processAdvertisement(Advertisement adv);
        };
    }

}

#endif // PROTOMESH_ROUTING_HPP
