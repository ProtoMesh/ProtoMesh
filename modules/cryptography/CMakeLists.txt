project(protoMesh::cryptography)

# Define the source files for this module
set(CRYPTOGRAPHY_SOURCES
        ${RESULT_SOURCES}
        ${ECC_SOURCES}
        ${PROJECT_SOURCE_DIR}/uuid.cpp
        ${PROJECT_SOURCE_DIR}/uuid.hpp
        ${PROJECT_SOURCE_DIR}/asymmetric.cpp
        ${PROJECT_SOURCE_DIR}/asymmetric.hpp
        ${PROJECT_SOURCE_DIR}/serialization.cpp
        ${PROJECT_SOURCE_DIR}/serialization.hpp
        ${PROJECT_SOURCE_DIR}/hash.cpp
        ${PROJECT_SOURCE_DIR}/hash.hpp
        ${PROJECT_SOURCE_DIR}/sha512.hpp)

# Add test files
set(PROTOMESH_TEST_FILES
        ${PROTOMESH_TEST_FILES}
        ${CRYPTOGRAPHY_SOURCES}
        PARENT_SCOPE)

# Add the required schemes as test dependencies
set(PROTOMESH_TEST_DEPS schemes_cryptography PARENT_SCOPE)
