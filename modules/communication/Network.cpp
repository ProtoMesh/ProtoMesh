#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Network.hpp"


namespace ProtoMesh::communication {

    Datagrams Network::processAdvertisement(const Datagram &datagram) {
        using namespace Routing::IARP;

        auto advertisementResult = Advertisement::fromBuffer(datagram);
        if (advertisementResult.isErr()) return {};

        Advertisement advertisement = advertisementResult.unwrap();

        if (advertisement.route.size() > ZONE_RADIUS) return {};

        this->routingTable.processAdvertisement(advertisement);
        advertisement.addHop(this->deviceID);

        // TODO Don't rebroadcast to the original sender. Some form of target node white/blacklist maybe
        //      Optionally obviously since not all transmission media might support it.
        return {advertisement.serialize()};
    }

    Datagrams Network::processRouteDiscovery(const Datagram &datagram) {
        using namespace Routing::IERP;

        auto routingResult = RouteDiscovery::fromBuffer(datagram);
        if (routingResult.isErr()) return {};

        RouteDiscovery routeDiscovery = routingResult.unwrap();

        vector<cryptography::UUID> bordercastNodes = this->routingTable.getBordercastNodes();
        return {};
    }

    Datagrams Network::processDeliveryFailure(const Datagram &datagram) {
        return {};
    }

    Datagrams Network::processMessageDatagram(const Datagram &datagram) {
        return {};
    }

    vector<vector<uint8_t>> Network::processDatagram(const Datagram &datagram) {
        using namespace flatbuffers;

        if (BufferHasIdentifier(datagram.data(), scheme::communication::iarp::AdvertisementDatagramIdentifier()))
            return processAdvertisement(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::ierp::RouteDiscoveryDatagramIdentifier()))
            return processRouteDiscovery(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::DeliveryFailureDatagramIdentifier()))
            return processDeliveryFailure(datagram);
        else if (BufferHasIdentifier(datagram.data(), scheme::communication::MessageDatagramIdentifier()))
            return processMessageDatagram(datagram);
        else
            return {};
    }

}