#ifdef UNIT_TESTING

#include "TransmissionHandler.hpp"
#include "Network.hpp"
#include "asymmetric.hpp"
#include "catch.hpp"

#endif

#include "Endpoint.hpp"

namespace ProtoMesh::interaction {

#ifdef UNIT_TESTING

    SCENARIO("An endpoint should be defined", "[unit_test][module][interaction]") {
        REL_TIME_PROV_T timeProvider(new DummyRelativeTimeProvider(0));
        TRANSMISSION_HANDLER_T transmissionHandler(new communication::transmission::NetworkStub());
        cryptography::UUID deviceID = cryptography::UUID();
        shared_ptr<communication::Network> network = make_shared<communication::Network>(deviceID, cryptography::asymmetric::generateKeyPair(), timeProvider);

        Endpoint<EndpointType::Metadata> meta(network, deviceID, 0);
        Endpoint<EndpointType::Authorization> auth(network, deviceID, 1);

        meta.getMetadata();

        REQUIRE(meta.type() == EndpointType::Metadata);
        REQUIRE(auth.type() == EndpointType::Authorization);

        /// -----

        std::vector<ENDPOINT_T> vec;

        vec.emplace_back(new Endpoint<EndpointType::Metadata>(network, deviceID, 0));
        vec.emplace_back(new Endpoint<EndpointType::Authorization>(network, deviceID, 1));

        for (ENDPOINT_T endpoint : vec) {
            switch (endpoint->type()) {
                case EndpointType::Metadata: {
                    auto metadata = dynamic_cast<Endpoint<EndpointType::Metadata> *>(endpoint.get());
                    metadata->getMetadata();
                    break;
                }
                case EndpointType::Color:break;
                case EndpointType::Temperature:break;
                case EndpointType::Brightness:break;
                case EndpointType::Authorization:break;
            }
        }
    }

#endif

}