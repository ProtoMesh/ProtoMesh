#ifndef PROTOMESH_ROUTING_HPP
#define PROTOMESH_ROUTING_HPP

#include <vector>
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
        vector<cryptography::UUID> bordercastNodes = {};
        REL_TIME_PROV_T timeProvider;
        uint zoneRadius;

        void deleteStaleBordercastNodes();

    public:
        explicit RoutingTable(REL_TIME_PROV_T timeProvider, uint zoneRadius = 2) : timeProvider(move(timeProvider)), zoneRadius(zoneRadius) {};

        Result<vector<cryptography::UUID>, RouteDiscoveryError> getRouteTo(cryptography::UUID uuid);

        void processAdvertisement(Advertisement adv);

        vector<cryptography::UUID> getBordercastNodes(vector<cryptography::UUID> nodesToExclude);
        vector<cryptography::UUID> getBordercastNodes();
    };

}

#endif // PROTOMESH_ROUTING_HPP
