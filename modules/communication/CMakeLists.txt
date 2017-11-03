project(protoMesh::communication)

# Define the source files for this module
set(COMMUNICATION_SOURCES
  ${PROJECT_SOURCE_DIR}/transmission.cpp
  ${PROJECT_SOURCE_DIR}/routing.cpp)

# Add test files
set(PROTOMESH_TEST_FILES
        ${PROTOMESH_TEST_FILES}
        ${COMMUNICATION_SOURCES}
        PARENT_SCOPE)

#create_flatbuffer_target(communication_buffers buffers)
#set(PROTOMESH_TEST_DEPS communication_buffers PARENT_SCOPE)
