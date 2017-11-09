#ifndef PROTOMESH_TRANSMISSION_HPP
#define PROTOMESH_TRANSMISSION_HPP

#include <vector>
#include <memory>

using namespace std;

namespace ProtoMesh::communication::transmission {
    enum class ReceiveResult {
        OK,
        NoData,
        Timeout
    };

    #define NETWORK_T shared_ptr<ProtoMesh::communication::transmission::Network>
    class Network {
    public:
        Network() = default;
        ~Network() = default;

        virtual void send(vector<uint8_t> message)= 0;
        virtual ReceiveResult recv(vector<uint8_t> *buffer, unsigned int timeout_ms)= 0;
    };

    class NetworkStub : public Network {
        vector<vector<uint8_t>> queue;
    public:

        void send(std::vector<uint8_t> message) override { queue.push_back(message); }
        ReceiveResult recv(std::vector<uint8_t>* buffer, unsigned int timeout_ms) override {
            if (queue.empty()) return ReceiveResult::NoData;
            *buffer = queue[0];
            queue.erase(queue.begin());
            return ReceiveResult::OK;
        }
    };
}

#endif // PROTOMESH_TRANSMISSION_HPP