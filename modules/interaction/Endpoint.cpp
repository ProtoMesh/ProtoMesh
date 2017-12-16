#ifdef UNIT_TESTING

#include "catch.hpp"

#endif

#include "Endpoint.hpp"

namespace ProtoMesh::interaction {

    template<EndpointType T>
    EndpointType Endpoint_Base<T>::type() { return T; }


#ifdef UNIT_TESTING

    SCENARIO("some scenario") {
        Endpoint<EndpointType::Metadata> test;

        test.someMetadataFunction();

        REQUIRE(test.type() == EndpointType::Metadata);
    }

#endif

}