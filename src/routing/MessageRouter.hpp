//
// Created by Til Blechschmidt on 22.10.17.
//

#ifndef PROTOMESH_MESSAGEROUTER_HPP
#define PROTOMESH_MESSAGEROUTER_HPP

#include <crypto/crypto.hpp>
#include <map>
#include "../buffers/network/route_discovery_datagram_generated.h"
#include "Route.hpp"

using namespace std;
using namespace protoMesh::network;

// TODO Add documentation for routes going stale and notifying all parties using the route that it went stale.

class MessageRouter {
private:
    /// Encryption related
    Crypto::UUID identifier;
    Crypto::asym::KeyPair key;

    /// Routing related
    map<Crypto::UUID, Route> zone;

    /// Dispatch a RouteDiscoveryDatagram
    void discoverRoute(Crypto::UUID destination);

    /// Process incoming datagrams
    void processDatagram(vector<uint8_t> datagram);
public:
    MessageRouter(Crypto::UUID identifier, Crypto::asym::KeyPair key) : identifier(identifier), key(key) {};

};


#endif //PROTOMESH_MESSAGEROUTER_HPP
