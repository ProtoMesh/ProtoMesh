#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Endpoint.hpp"

namespace ProtoMesh::interaction {

#ifdef UNIT_TESTING

    SCENARIO("some scenario") {
        Endpoint<EndpointType::Metadata> meta;
        Endpoint<EndpointType::Authorization> auth;

        meta.someMetadataFunction();

        REQUIRE(meta.type() == EndpointType::Metadata);
        REQUIRE(auth.type() == EndpointType::Authorization);

        /// -----

        std::vector<ENDPOINT_T> vec;

        vec.emplace_back(new Endpoint<EndpointType::Metadata>());
        vec.emplace_back(new Endpoint<EndpointType::Authorization>());

        for (ENDPOINT_T endpoint : vec) {
            switch (endpoint->type()) {

                case EndpointType::Metadata: {
                    auto metadata = dynamic_cast<Endpoint<EndpointType::Metadata> *>(endpoint.get());
                    metadata->someMetadataFunction();
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