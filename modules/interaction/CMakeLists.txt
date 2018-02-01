project(module_interaction)

# Define the source files and dependencies for this module
set(INTERACTION_DEPENDENCIES schemes_interaction)
set(INTERACTION_SOURCES
        ${PROJECT_SOURCE_DIR}/rpc/FunctionCall.cpp
        ${PROJECT_SOURCE_DIR}/rpc/FunctionCall.hpp
        ${PROJECT_SOURCE_DIR}/rpc/FunctionCallResponse.cpp
        ${PROJECT_SOURCE_DIR}/rpc/FunctionCallResponse.hpp
        ${PROJECT_SOURCE_DIR}/variable_types.hpp
        ${PROJECT_SOURCE_DIR}/Endpoint.cpp
        ${PROJECT_SOURCE_DIR}/Endpoint.hpp
        ${PROJECT_SOURCE_DIR}/EndpointTypes/Brightness.cpp
        ${PROJECT_SOURCE_DIR}/EndpointTypes/Brightness.hpp)

# Add library target
add_library(${PROJECT_NAME} STATIC ${INTERACTION_SOURCES})
add_dependencies(${PROJECT_NAME} ${INTERACTION_DEPENDENCIES})


# Add test files
set(PROTOMESH_TEST_FILES
        ${PROTOMESH_TEST_FILES}
        ${INTERACTION_SOURCES}
        PARENT_SCOPE)

# Add the required schemes as test dependencies
set(PROTOMESH_TEST_DEPS
        ${PROTOMESH_TEST_DEPS}
        ${INTERACTION_DEPENDENCIES}
        PARENT_SCOPE)
