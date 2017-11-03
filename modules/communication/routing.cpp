#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

#include "routing.hpp"

#ifdef UNIT_TESTING
SCENARIO("Two nodes within the same zone should communicate") {

    GIVEN("A network stub and two devices") {
        NETWORK_T stub = make_shared<Transmission::NetworkStub>();

        WHEN("one node sends an advertisement") {
//            stub->send(IARP::Advertisement().serialize());
        }
    }
}

#endif // UNIT_TESTING