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

        std::vector<ENDPOINT_T> vec;

//        vec.push_back(make_shared<Endpoint<EndpointType::Metadata>>(network, deviceID, 0));
        vec.push_back(make_shared<BrightnessEndpoint>(network, deviceID, 1));

        ENDPOINT_T brightness_endpoint = make_shared<BrightnessEndpoint>(network, deviceID, 1);

        for (ENDPOINT_T endpoint : vec) {
            switch (endpoint->type()) {
                case EndpointType::Brightness: {
                    auto brightness = (BrightnessEndpoint *) brightness_endpoint.get();
                    brightness->getBrightness(RequestType::GET);
                    break;
                }
                case EndpointType::Metadata:break;
                case EndpointType::Temperature:break;
            }
        }
    }

#endif

}