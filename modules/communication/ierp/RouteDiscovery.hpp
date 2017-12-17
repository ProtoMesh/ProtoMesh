#ifndef PROTOMESH_ROUTEDISCOVERY_HPP
#define PROTOMESH_ROUTEDISCOVERY_HPP

#include <utility>

#include "Serializable.hpp"
#include "RelativeTimeProvider.hpp"
#include "uuid.hpp"
#include "asymmetric.hpp"

#include "flatbuffers/flatbuffers.h"
#include "communication/ierp/routeDiscovery_generated.h"

namespace ProtoMesh::communication::Routing::IERP {

    class RouteDiscovery : public Serializable<RouteDiscovery> {
    public:
        vector<cryptography::UUID> coveredNodes;

        cryptography::asymmetric::PublicKey origin;
        cryptography::UUID destination;

        vector<cryptography::UUID> route;

        long sentTimestamp;

        RouteDiscovery(cryptography::asymmetric::PublicKey origin, cryptography::UUID destination, long sentTimestamp,
                       vector<cryptography::UUID> route, vector<cryptography::UUID> coveredNodes)
                : coveredNodes(std::move(coveredNodes)), origin(origin), destination(destination),
                  route(std::move(route)), sentTimestamp(sentTimestamp) {}

        void addHop(cryptography::UUID hop);
        void addCoveredNodes(const vector<cryptography::UUID> &nodes);

        static inline RouteDiscovery
        discover(cryptography::UUID target, cryptography::asymmetric::PublicKey selfKey, cryptography::UUID self,
                 long timestamp) {
            return RouteDiscovery(selfKey, target, timestamp, {self}, {self});
        };

        /// Serializable overrides
        static Result<RouteDiscovery, DeserializationError> fromBuffer(vector<uint8_t> buffer);
        vector<uint8_t> serialize() const;
    };

}


#endif //PROTOMESH_ROUTEDISCOVERY_HPP
