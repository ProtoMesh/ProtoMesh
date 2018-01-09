#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

#include "TransmissionHandler.hpp"

namespace ProtoMesh::communication::transmission {

#ifdef UNIT_TESTING

    SCENARIO("A network stub is required for unit testing", "[unit_test][module][communication][transmission][stub]") {
        GIVEN("A network instance and a message") {
            TRANSMISSION_HANDLER_T stub = make_shared<NetworkStub>();
            vector<uint8_t> msg = {1, 2, 3, 4, 5};

            WHEN("a single message is sent") {
                stub->send(msg);
                THEN("it should be receivable again") {
                    vector<uint8_t> buf;
                    REQUIRE(stub->recv(&buf, 1000) == ReceiveResult::OK);
                    REQUIRE(msg == buf);

                    AND_THEN("there should be no more receivable messages") {
                        REQUIRE(stub->recv(&buf, 1000) == ReceiveResult::NoData);
                    }
                }
            }
        }
    }

#endif // UNIT_TESTING

}