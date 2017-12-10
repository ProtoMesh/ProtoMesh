#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "RouteCache.hpp"

namespace ProtoMesh::communication::Routing::IERP {

    Result<RouteCacheEntry, RouteCache::RouteCacheError> RouteCache::getRouteTo(cryptography::UUID uuid) {
        if (routes.find(uuid) != routes.end()) {
            vector<RouteCacheEntry> &availableRoutes = routes.at(uuid);

            size_t routeIndex = 0;
            size_t routeLength = 9999;

            for (size_t i = 0; i < availableRoutes.size(); i++) {
                RouteCacheEntry route = availableRoutes[i];

                if (route.route.size() < routeLength) {
                    routeLength = route.route.size();
                    routeIndex = i;
                }
            }

            return Ok(availableRoutes[routeIndex]);
        } else
            return Err(RouteCacheError::NO_ROUTE_AVAILABLE);
    }

    void RouteCache::addRoute(cryptography::UUID destination, vector<cryptography::UUID> route) {

        /// Check if we already have a route for this target
        if (routes.find(destination) != routes.end()) {
            // Possibly add the new route entry
            vector<RouteCacheEntry> &availableRoutes = routes.at(destination);
            availableRoutes.emplace_back(route, 0);
        } else {
            /// Insert the route
            vector<RouteCacheEntry> availableRoutes({RouteCacheEntry(route, 0)});
            routes.insert({destination, availableRoutes});
        }
    }

#ifdef UNIT_TESTING

    SCENARIO("Discovered routes should be cached",
             "[unit_test][module][communication][routing][ierp]") {
        GIVEN("A RouteCache and a route") {
            RouteCache routeCache;
            cryptography::UUID hop1; // us
            cryptography::UUID hop2;
            cryptography::UUID hop3; // destination
            vector<cryptography::UUID> route = {hop1, hop2, hop3};

            THEN("requesting the route should result in an error") {
                REQUIRE(routeCache.getRouteTo(hop3).isErr());
            }

            WHEN("the route is added to the cache") {
                routeCache.addRoute(hop3, route);

                THEN("retrieving the route should yield the original route") {
                    REQUIRE(routeCache.getRouteTo(hop3).unwrap().route == route);
                }
            }
        }
    }

#endif
}