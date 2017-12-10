#ifndef PROTOMESH_NETWORK_HPP
#define PROTOMESH_NETWORK_HPP

#include <algorithm>
#include <utility>
#include <vector>
#include <tuple>
#include <list>
#include <ierp/RouteCache.hpp>

using namespace std;

#include "iarp/RoutingTable.hpp"
#include "iarp/Advertisement.hpp"
#include "ierp/RouteDiscovery.hpp"
#include "ierp/RouteCache.hpp"
#include "Message.hpp"
#include "CredentialsStore.hpp"

#include "flatbuffers/flatbuffers.h"
#include "communication/message_generated.h"
#include "communication/deliveryFailure_generated.h"
#include "communication/iarp/advertisement_generated.h"
#include "communication/ierp/routeDiscovery_generated.h"
#include "communication/ierp/routeDiscoveryAcknowledgement_generated.h"

#define Datagram vector<uint8_t>
#define Datagrams vector<tuple<MessageTarget, Datagram>>

// Note that the route length is defined in zones so the actual hop count would be MAXIMUM_ROUTE_LENGTH * ZONE_RADIUS
#define MAXIMUM_ROUTE_LENGTH 20
#define ZONE_RADIUS 2

namespace ProtoMesh::communication {

    class MessageTarget {
        enum class Type {
            SINGLE,
            BROADCAST
        };

        Type type;
        cryptography::UUID target;

        explicit MessageTarget(Type type, cryptography::UUID target = cryptography::UUID::Empty())
                : type(type), target(target) {};

    public:
        static MessageTarget broadcast() {
            return MessageTarget(Type::BROADCAST);
        }

        static MessageTarget single(cryptography::UUID target) {
            return MessageTarget(Type::SINGLE, target);
        }
    };



    class Network {
        cryptography::UUID deviceID;
        cryptography::asymmetric::KeyPair deviceKeys;
        Routing::IARP::RoutingTable routingTable;
        Routing::IERP::RouteCache routeCache;

        CredentialsStore credentials;

        /// Datagram processing
        Datagrams processAdvertisement(const Datagram &datagram);
        Datagrams processRouteDiscovery(const Datagram &datagram);
        Datagrams processRouteDiscoveryAcknowledgement(const Datagram &datagram);
        Datagrams processDeliveryFailure(const Datagram &datagram);
        Datagrams processMessageDatagram(const Datagram &datagram);

        /// Helpers
        Datagrams rebroadcastRouteDiscovery(Routing::IERP::RouteDiscovery routeDiscovery);
        Datagrams dispatchRouteDiscoveryAcknowledgement(Routing::IERP::RouteDiscovery routeDiscovery);

    public:
        explicit Network(cryptography::UUID deviceID, cryptography::asymmetric::KeyPair deviceKeys, REL_TIME_PROV_T timeProvider)
                : deviceID(deviceID), deviceKeys(deviceKeys), routingTable(std::move(timeProvider), ZONE_RADIUS) {};

        Datagrams processDatagram(const Datagram &datagram);
    };

}


#endif //PROTOMESH_NETWORK_HPP
