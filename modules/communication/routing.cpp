#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

#include "routing.hpp"

namespace ProtoMesh::communication::Routing {

#ifdef UNIT_TESTING

    SCENARIO("Two nodes within the same zone should communicate") {

        GIVEN("A routing table and a advertisement byte stream") {
//        NETWORK_T stub = make_shared<Transmission::NetworkStub>();
            cryptography::UUID uuid;
            Routing::IARP::Advertisement adv(uuid);
            Routing::IARP::RoutingTable table;

            WHEN("the advertisement is added to the routing table") {
                table.processAdvertisement(adv.serialize());
                THEN("a route to the advertiser should be available") {
//                    table.getRouteTo(uuid);
                }
            }
        }
    }

#endif // UNIT_TESTING

}