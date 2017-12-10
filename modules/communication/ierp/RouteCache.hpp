#ifndef PROTOMESH_ROUTECACHE_HPP
#define PROTOMESH_ROUTECACHE_HPP

#include <utility>

#include <unordered_map>
#include "asymmetric.hpp"
#include "uuid.hpp"

namespace ProtoMesh::communication::Routing::IERP {

    class RouteCacheEntry {
    public:
        long validUntil;
        vector<cryptography::UUID> route;

        RouteCacheEntry(vector<cryptography::UUID> route, long validUntil)
                : validUntil(validUntil), route(std::move(route)) {}
    };

    class RouteCache {
        unordered_map<cryptography::UUID, vector<RouteCacheEntry>> routes;

    public:
        enum class RouteCacheError {
            NO_ROUTE_AVAILABLE
        };

        void addRoute(cryptography::UUID destination, vector<cryptography::UUID> route);
        Result<RouteCacheEntry, RouteCacheError> getRouteTo(cryptography::UUID uuid);
    };

}


#endif //PROTOMESH_ROUTECACHE_HPP
