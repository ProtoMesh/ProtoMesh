#include "network.hpp"

#ifdef UNIT_TESTING
    SCENARIO("API/network/dummy", "[network]") {
        GIVEN("A dummy network handler") {
            DummyNetworkHandler net;

            GIVEN("A dummy broadcast socket") {
                BCAST_SOCKET_T bcast = net.createBroadcastSocket(MULTICAST_NETWORK, REGISTRY_PORT);

                WHEN("A message is broadcasted") {
                    std::vector<uint8_t> vec({1, 2, 3, 4});
                    bcast->broadcast(vec);

                    THEN("Receiving should return the very same message") {
                        std::vector<uint8_t> received;
                        REQUIRE(bcast->recv(&received, 0) == RECV_OK);
                        REQUIRE(vec == received);

                        AND_THEN("Receiving again should yield an error") {
                            std::vector<uint8_t> received2;
                            REQUIRE(bcast->recv(&received2, 0) == RECV_ERR);
                        }
                    }
                }

                WHEN("Two messages are broadcasted") {
                    std::vector<uint8_t> vec({1, 2, 3, 4});
                    std::vector<uint8_t> vec2({5, 6, 7, 8});
                    bcast->broadcast(vec);
                    bcast->broadcast(vec2);

                    THEN("Receiving should return the first added message") {
                        std::vector<uint8_t> received;
                        REQUIRE(bcast->recv(&received, 0) == RECV_OK);
                        REQUIRE(vec == received);

                        AND_THEN("Receiving again should return the second message") {
                            std::vector<uint8_t> received2;
                            REQUIRE(bcast->recv(&received2, 0) == RECV_OK);
                            REQUIRE(vec2 == received2);
                        }
                    }
                }
            }

        }
    }
#endif