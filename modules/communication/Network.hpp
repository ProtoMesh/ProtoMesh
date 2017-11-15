#ifndef PROTOMESH_NETWORK_HPP
#define PROTOMESH_NETWORK_HPP

#include <utility>
#include <vector>
#include <list>

using namespace std;

#include "iarp/RoutingTable.hpp"
#include "iarp/Advertisement.hpp"
#include "ierp/RouteDiscovery.hpp"

#include "flatbuffers/flatbuffers.h"
#include "communication/message_generated.h"
#include "communication/deliveryFailure_generated.h"
#include "communication/iarp/advertisement_generated.h"
#include "communication/ierp/routeDiscovery_generated.h"

#define Datagram vector<uint8_t>
#define Datagrams vector<Datagram>

#define ZONE_RADIUS 2

namespace ProtoMesh::communication {

    class Network {
        cryptography::UUID deviceID;
        Routing::IARP::RoutingTable routingTable;

        Datagrams processAdvertisement(const Datagram &datagram);

        Datagrams processRouteDiscovery(const Datagram &datagram);

        Datagrams processDeliveryFailure(const Datagram &datagram);

        Datagrams processMessageDatagram(const Datagram &datagram);

    public:
        explicit Network(cryptography::UUID deviceID, REL_TIME_PROV_T timeProvider)
                : deviceID(deviceID), routingTable(std::move(timeProvider), ZONE_RADIUS) {};

        Datagrams processDatagram(const Datagram &datagram);
    };

}


#endif //PROTOMESH_NETWORK_HPP
