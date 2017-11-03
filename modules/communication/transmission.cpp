#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

#include <vector>

using namespace std;

namespace ProtoMesh::Communication::Transmission {
    enum class ReceiveResult {
        OK,
        NoData,
        Timeout
    };

    #define NETWORK_T shared_ptr<Network>
    class Network {
    public:
        Network() = default;
        ~Network() = default;

        virtual void send(vector<uint8_t> message)= 0;
        virtual ReceiveResult recv(vector<uint8_t> *buffer, unsigned int timeout_ms)= 0;
    };



#ifdef UNIT_TESTING
    class NetworkStub : public Network {
        vector<vector<uint8_t>> queue;
    public:

        void send(std::vector<uint8_t> message) override {
            queue.push_back(message);
        }

        ReceiveResult recv(std::vector<uint8_t>* buffer, unsigned int timeout_ms) override {
            if (queue.empty()) return ReceiveResult::NoData;
            *buffer = queue[0];
            queue.erase(queue.begin());
            return ReceiveResult::OK;
        }
    };

    SCENARIO("A transmission stub is required for unit testing", "[module][communication][transmission][stub]") {
        GIVEN("A transmission stub instance and a message") {
            NETWORK_T stub = make_shared<NetworkStub>();
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