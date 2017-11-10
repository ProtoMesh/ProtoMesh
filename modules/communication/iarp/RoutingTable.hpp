#ifndef PROTOMESH_ROUTING_HPP
#define PROTOMESH_ROUTING_HPP

#include <utility>
#include <unordered_map>
#include <RelativeTimeProvider.hpp>

#include "Advertisement.hpp"

using namespace std;
using namespace ProtoMesh::communication;

namespace ProtoMesh::communication::Routing::IARP {

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
        REL_TIME_PROV_T timeProvider;

    public:
        RoutingTable(REL_TIME_PROV_T timeProvider) : timeProvider(timeProvider) {};

        Result<vector<cryptography::UUID>, RouteDiscoveryError> getRouteTo(cryptography::UUID uuid);

        void processAdvertisement(Advertisement adv);
    };

}

#endif // PROTOMESH_ROUTING_HPP
