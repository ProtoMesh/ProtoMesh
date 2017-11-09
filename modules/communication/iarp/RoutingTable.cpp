#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "RoutingTable.hpp"

namespace ProtoMesh::communication::Routing {

    Result<vector<cryptography::UUID>, IARP::RouteDiscoveryError>
    IARP::RoutingTable::getRouteTo(cryptography::UUID uuid) {
        if (routes.find(uuid) != routes.end()) {
            vector<RoutingTableEntry> &availableRoutes = routes.at(uuid);

            uint routeIndex = -1;
            uint routeLength = 9999;
            for (uint i = 0; i < availableRoutes.size(); i++) {
                RoutingTableEntry route = availableRoutes[i];
                // TODO Check for validity of RoutingTableEntry and if it isn't valid anymore delete it.
                if (route.route.size() < routeLength) {
                    routeLength = route.route.size();
                    routeIndex = i;
                }
            }
            return Ok(availableRoutes[routeIndex].route);
        } else
            return Err(RouteDiscoveryError::NO_ROUTE_AVAILABLE);
    }

    void IARP::RoutingTable::processAdvertisement(IARP::Advertisement adv) {
        // TODO Replace this with the real thing
        long currentTime = 0;

        /// Check if we already have a route for this target
        if (routes.find(adv.uuid) != routes.end()) {
            // Possibly add the new route entry
            vector<RoutingTableEntry> &availableRoutes = routes.at(adv.uuid);
            availableRoutes.push_back(RoutingTableEntry(adv, currentTime));
        }

        /// Insert the route
        vector<RoutingTableEntry> availableRoutes({RoutingTableEntry(adv, currentTime)});
        routes.insert({adv.uuid, availableRoutes});
    }

#ifdef UNIT_TESTING

    SCENARIO("The best route between two devices should be discovered",
             "[unit_test][module][communication][routing][iarp]") {

        GIVEN("A routing table and a advertisement") {
            cryptography::UUID uuid;
            cryptography::UUID hop1;
            cryptography::UUID hop2;
            cryptography::UUID hop3;
            cryptography::asymmetric::KeyPair pair(cryptography::asymmetric::generateKeyPair());
            Routing::IARP::Advertisement adv = IARP::Advertisement::build(uuid, pair);
            Routing::IARP::Advertisement adv2 = IARP::Advertisement::build(uuid, pair);
            Routing::IARP::RoutingTable table;

            adv.addHop(hop1);
            adv.addHop(hop2);
            adv.addHop(hop3);

            WHEN("the advertisement is added to the routing table") {
                table.processAdvertisement(adv);
                THEN("a route to the advertiser should be available") {
                    vector<cryptography::UUID> route = table.getRouteTo(uuid).unwrap();
                    vector<cryptography::UUID> expectedRoute({hop3, hop2, hop1});
                    REQUIRE(route == expectedRoute);
                }

                AND_WHEN("a second advertisement with three hops is added to the routing table") {
                    adv2.addHop(hop1);
                    adv2.addHop(hop2);
                    table.processAdvertisement(adv2);

                    THEN("the route to the advertiser should be the shorter of the two") {
                        vector<cryptography::UUID> route = table.getRouteTo(uuid).unwrap();
                        vector<cryptography::UUID> expectedRoute({hop2, hop1});
                        REQUIRE(route == expectedRoute);
                    }
                }
            }
        }
    }

#endif // UNIT_TESTING
}
