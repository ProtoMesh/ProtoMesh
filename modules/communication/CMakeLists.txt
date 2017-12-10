project(module_communication)

# Define the source files and dependencies for this module
set(COMMUNICATION_DEPENDENCIES schemes_communication)
set(COMMUNICATION_SOURCES
        ${PROJECT_SOURCE_DIR}/Message.cpp
        ${PROJECT_SOURCE_DIR}/Message.hpp
        ${PROJECT_SOURCE_DIR}/Network.cpp
        ${PROJECT_SOURCE_DIR}/Network.hpp
        ${PROJECT_SOURCE_DIR}/CredentialsStore.cpp
        ${PROJECT_SOURCE_DIR}/CredentialsStore.hpp
        ${PROJECT_SOURCE_DIR}/TransmissionHandler.cpp
        ${PROJECT_SOURCE_DIR}/TransmissionHandler.hpp
        ${PROJECT_SOURCE_DIR}/iarp/RoutingTable.cpp
        ${PROJECT_SOURCE_DIR}/iarp/RoutingTable.hpp
        ${PROJECT_SOURCE_DIR}/iarp/Advertisement.cpp
        ${PROJECT_SOURCE_DIR}/iarp/Advertisement.hpp
        ${PROJECT_SOURCE_DIR}/ierp/RouteDiscovery.cpp
        ${PROJECT_SOURCE_DIR}/ierp/RouteDiscovery.hpp
        ${PROJECT_SOURCE_DIR}/ierp/RouteCache.cpp
        ${PROJECT_SOURCE_DIR}/ierp/RouteCache.hpp
        ${PROJECT_SOURCE_DIR}/RelativeTimeProvider.hpp)

# Add library target
add_library(${PROJECT_NAME} STATIC ${COMMUNICATION_SOURCES})
add_dependencies(${PROJECT_NAME} ${COMMUNICATION_DEPENDENCIES})


# Add test files
set(PROTOMESH_TEST_FILES
        ${PROTOMESH_TEST_FILES}
        ${COMMUNICATION_SOURCES}
        PARENT_SCOPE)

# Add the required schemes as test dependencies
set(PROTOMESH_TEST_DEPS
        ${PROTOMESH_TEST_DEPS}
        ${COMMUNICATION_DEPENDENCIES}
        PARENT_SCOPE)
