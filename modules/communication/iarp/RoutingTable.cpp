#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "RoutingTable.hpp"

namespace ProtoMesh::communication::Routing {

    Result<vector<cryptography::UUID>, IARP::RouteDiscoveryError>
    IARP::RoutingTable::getRouteTo(cryptography::UUID uuid) {
        if (routes.find(uuid) != routes.end()) {
            vector<RoutingTableEntry> &availableRoutes = routes.at(uuid);

            size_t routeIndex = 0;
            size_t routeLength = 9999;
            long currentTime = this->timeProvider->millis();

            vector<size_t> staleRoutes;
            for (size_t i = 0; i < availableRoutes.size(); i++) {
                RoutingTableEntry route = availableRoutes[i];

                if (route.validUntil < currentTime) {
                    staleRoutes.push_back(i);
                    continue;
                }

                if (route.route.size() < routeLength) {
                    routeLength = route.route.size();
                    routeIndex = i;
                }
            }

            reverse(staleRoutes.begin(), staleRoutes.end());

            if (staleRoutes.size() == availableRoutes.size()) {
                /// Delete the whole entry in the map
                routes.erase(uuid);
                return Err(RouteDiscoveryError::NO_ROUTE_AVAILABLE);
            } else {
                /// Delete only the stale routes but keep the vector
                for (size_t staleRoute : staleRoutes)
                    availableRoutes.erase(availableRoutes.begin() + staleRoute);
            }

            return Ok(availableRoutes[routeIndex].route);
        } else
            return Err(RouteDiscoveryError::NO_ROUTE_AVAILABLE);
    }

    void IARP::RoutingTable::processAdvertisement(IARP::Advertisement adv) {
        long currentTime = this->timeProvider->millis();

        /// Check if we already have a route for this target
        if (routes.find(adv.uuid) != routes.end()) {
            // Possibly add the new route entry
            vector<RoutingTableEntry> &availableRoutes = routes.at(adv.uuid);
            availableRoutes.emplace_back(adv, currentTime);
        }

        /// Insert the route
        vector<RoutingTableEntry> availableRoutes({RoutingTableEntry(adv, currentTime)});
        routes.insert({adv.uuid, availableRoutes});
    }

#ifdef UNIT_TESTING

    SCENARIO("The shortest route between two devices should be discovered",
             "[unit_test][module][communication][routing][iarp]") {

        GIVEN("A routing table and a advertisement") {
            cryptography::UUID uuid;
            cryptography::UUID hop1;
            cryptography::UUID hop2;
            cryptography::UUID hop3;

            cryptography::asymmetric::KeyPair pair(cryptography::asymmetric::generateKeyPair());

            Routing::IARP::Advertisement adv = IARP::Advertisement::build(uuid, pair);
            Routing::IARP::Advertisement adv2 = IARP::Advertisement::build(uuid, pair);

            REL_TIME_PROV_T timeProvider(new DummyRelativeTimeProvider(0));
            Routing::IARP::RoutingTable table(timeProvider);

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

                AND_WHEN("the time advances by 20000ms") {
                    ((DummyRelativeTimeProvider *) timeProvider.get())->turnTheClockBy(20000);

                    THEN("the route should've gone stale and no route should be available") {
                        REQUIRE(table.getRouteTo(uuid).isErr());
                    }
                }

                AND_WHEN("the time advances by 6000ms, the same advertisement is received again") {
                    ((DummyRelativeTimeProvider *) timeProvider.get())->turnTheClockBy(6000);
                    table.processAdvertisement(adv);

                    AND_WHEN("the time advances by another 5000ms") {
                        ((DummyRelativeTimeProvider *) timeProvider.get())->turnTheClockBy(5000);

                        THEN("a route should be available") {
                            REQUIRE(table.getRouteTo(uuid).isOk());
                        }

                    }

                    AND_WHEN("the time advances by another 20000ms") {
                        ((DummyRelativeTimeProvider *) timeProvider.get())->turnTheClockBy(20000);
                        THEN("both routes should've gone stale") {
                            REQUIRE(table.getRouteTo(uuid).isErr());
                        }
                    }
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
