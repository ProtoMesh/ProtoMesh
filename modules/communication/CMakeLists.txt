project(protoMesh_module_network)

# Add test files
set(PROTOMESH_TEST_FILES
        ${PROTOMESH_TEST_FILES}
        ${PROJECT_SOURCE_DIR}/transmission.cpp
        PARENT_SCOPE)

create_flatbuffer_target(network_buffers buffers)
set(PROTOMESH_TEST_DEPS network_buffers PARENT_SCOPE)