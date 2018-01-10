project(module_integration)

# Define the source files and dependencies for this module
set(INTEGRATION_SOURCES
        ${PROJECT_SOURCE_DIR}/MeshHandler.cpp
        ${PROJECT_SOURCE_DIR}/MeshHandler.hpp
        ${PROJECT_SOURCE_DIR}/Device.cpp
        ${PROJECT_SOURCE_DIR}/Device.hpp
        ${PROJECT_SOURCE_DIR}/delegates/DeviceHandlerDelegate.hpp
        ${PROJECT_SOURCE_DIR}/delegates/DeviceDelegate.hpp)

# Add library target
add_library(${PROJECT_NAME} STATIC ${INTEGRATION_SOURCES})


# Add test files
set(PROTOMESH_TEST_FILES
        ${PROTOMESH_TEST_FILES}
        ${INTEGRATION_SOURCES}
        PARENT_SCOPE)

# Add the required schemes as test dependencies
set(PROTOMESH_TEST_DEPS
        ${PROTOMESH_TEST_DEPS}
        ${INTEGRATION_DEPENDENCIES}
        PARENT_SCOPE)
