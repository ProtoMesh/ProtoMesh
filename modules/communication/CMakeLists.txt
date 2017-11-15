project(protoMesh::communication)

# Define the source files for this module
set(COMMUNICATION_SOURCES
        ${PROJECT_SOURCE_DIR}/transmission.cpp
        ${PROJECT_SOURCE_DIR}/transmission.hpp
        ${PROJECT_SOURCE_DIR}/iarp/RoutingTable.cpp
        ${PROJECT_SOURCE_DIR}/iarp/RoutingTable.hpp
        ${PROJECT_SOURCE_DIR}/iarp/Advertisement.cpp
        ${PROJECT_SOURCE_DIR}/iarp/Advertisement.hpp
        ${PROJECT_SOURCE_DIR}/ierp/RouteDiscovery.hpp
        ${PROJECT_SOURCE_DIR}/ierp/RouteDiscovery.cpp
        ${PROJECT_SOURCE_DIR}/RelativeTimeProvider.hpp)

# Add test files
set(PROTOMESH_TEST_FILES
        ${PROTOMESH_TEST_FILES}
        ${COMMUNICATION_SOURCES}
        PARENT_SCOPE)

# Add the required schemes as test dependencies
set(PROTOMESH_TEST_DEPS
        ${PROTOMESH_TEST_DEPS}
        schemes_communication
        PARENT_SCOPE)
