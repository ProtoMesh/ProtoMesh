#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "RoutingTable.hpp"

namespace ProtoMesh::communication::Routing::IARP {

    Result<RoutingTableEntry, RouteDiscoveryError>
    RoutingTable::getRouteTo(cryptography::UUID uuid) {
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

            return Ok(availableRoutes[routeIndex]);
        } else
            return Err(RouteDiscoveryError::NO_ROUTE_AVAILABLE);
    }

    void RoutingTable::processAdvertisement(Advertisement adv) {
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

        /// Store the nodeID as a bordercast node if it is zoneRadius hops away
        /// Since the route in the advertisement does not contain the advertiser add one to the size
        if (adv.route.size()+1 == zoneRadius) this->bordercastNodes.push_back(adv.uuid);
    }

    void RoutingTable::deleteStaleBordercastNodes() {
        vector<size_t> staleNodes;
        for (size_t i = 0; i < this->bordercastNodes.size(); ++i)
            if (this->getRouteTo(this->bordercastNodes[i]).isErr())
                staleNodes.push_back(i);

        reverse(staleNodes.begin(), staleNodes.end());

        for (auto staleNodeIndex : staleNodes)
            this->bordercastNodes.erase(this->bordercastNodes.begin() + staleNodeIndex);
    }

    vector<cryptography::UUID> RoutingTable::getBordercastNodes(vector<cryptography::UUID> nodesToExclude) {
        this->deleteStaleBordercastNodes();

        vector<cryptography::UUID> resultingNodes;

        for (auto bordercastNode : this->bordercastNodes) {
            for (auto excludedNode : nodesToExclude) {
                if (bordercastNode == excludedNode) goto outerLoop;
            }

            resultingNodes.push_back(bordercastNode);

            outerLoop:;
        }

        return resultingNodes;
    }

    vector<cryptography::UUID> RoutingTable::getBordercastNodes() {
        return this->getBordercastNodes({});
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

            Advertisement adv = Advertisement::build(uuid, pair);
            Advertisement adv2 = Advertisement::build(uuid, pair);

            REL_TIME_PROV_T timeProvider(new DummyRelativeTimeProvider(0));
            RoutingTable table(timeProvider);

            adv.addHop(hop1);
            adv.addHop(hop2);
            adv.addHop(hop3);

            WHEN("the advertisement is added to the routing table") {
                table.processAdvertisement(adv);
                THEN("a route to the advertiser should be available") {
                    vector<cryptography::UUID> route = table.getRouteTo(uuid).unwrap().route;
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
                        vector<cryptography::UUID> route = table.getRouteTo(uuid).unwrap().route;
                        vector<cryptography::UUID> expectedRoute({hop2, hop1});
                        REQUIRE(route == expectedRoute);
                    }
                }
            }
        }
    }

    SCENARIO("The routing table should keep track of bordercast nodes",
             "[unit_test][module][communication][routing][ierp]") {
        GIVEN("Multiple advertisements with different route lengths and a routing table") {
            // Network topology (we are E):
            //  A              B
            //      C       D
            //          E
            //      F       G
            //  H               I
            cryptography::UUID a, b, c, d, f, g, h, i;
            cryptography::asymmetric::KeyPair pair(cryptography::asymmetric::generateKeyPair());
            Advertisement adv_a = Advertisement::build(a, pair);
            Advertisement adv_b = Advertisement::build(b, pair);
            Advertisement adv_c = Advertisement::build(c, pair);
            Advertisement adv_d = Advertisement::build(d, pair);
            Advertisement adv_f = Advertisement::build(f, pair);
            Advertisement adv_g = Advertisement::build(g, pair);
            Advertisement adv_h = Advertisement::build(h, pair);
            Advertisement adv_i = Advertisement::build(i, pair);

            adv_a.addHop(c);
            adv_b.addHop(d);
            adv_h.addHop(f);
            adv_i.addHop(g);

            REL_TIME_PROV_T timeProvider(new DummyRelativeTimeProvider(0));
            RoutingTable table(timeProvider);

            WHEN("the advertisements are added to the table") {
                table.processAdvertisement(adv_a);
                table.processAdvertisement(adv_b);
                table.processAdvertisement(adv_c);
                table.processAdvertisement(adv_d);
                table.processAdvertisement(adv_f);
                table.processAdvertisement(adv_g);
                table.processAdvertisement(adv_h);
                table.processAdvertisement(adv_i);

                THEN("it should output the bordercast nodes for the default zone radius of 2") {
                    vector<cryptography::UUID> bordercastNodes = table.getBordercastNodes();
                    vector<cryptography::UUID> expectedNodes = {a, b, h, i};
                    REQUIRE(bordercastNodes == expectedNodes);
                }

                GIVEN("a list of covered nodes consisting of a, b") {
                    vector<cryptography::UUID> coveredNodes = {a, b};

                    THEN("the list of bordercast nodes should contain neither a or b") {
                        vector<cryptography::UUID> bordercastNodes = table.getBordercastNodes(coveredNodes);
                        vector<cryptography::UUID> expectedNodes = {h, i};
                        REQUIRE(bordercastNodes == expectedNodes);
                    }
                }

                AND_WHEN("the time advances by 20000ms") {
                    ((DummyRelativeTimeProvider *) timeProvider.get())->turnTheClockBy(20000);
                    THEN("add bordercast nodes should've been removed") {
                        REQUIRE(table.getBordercastNodes().empty());
                    }
                }
            }
        }
    }

#endif // UNIT_TESTING
}
