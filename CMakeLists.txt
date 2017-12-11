cmake_minimum_required(VERSION 3.7)
project(protoMesh)

# C++ compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Wno-unused-parameter")

# Enable the use of the global offset table
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include CMake helpers
include(cmake/BuildFlatbuffers.cmake)

# Include unit testing framework and init relevant variables
include_directories(lib/catch/single_include)
include_directories(${CMAKE_SOURCE_DIR}/modules/testing)
set(PROTOMESH_TEST_FILES ${CMAKE_SOURCE_DIR}/modules/testing/test.cpp)
set(PROTOMESH_TEST_DEPS)

# Add micro-ecc sources
include_directories(lib/micro-ecc)
set(ECC_SOURCES
        ${CMAKE_SOURCE_DIR}/lib/micro-ecc/uECC.h
        ${CMAKE_SOURCE_DIR}/lib/micro-ecc/uECC.c
        ${CMAKE_SOURCE_DIR}/lib/micro-ecc/uECC_vli.h
        ${CMAKE_SOURCE_DIR}/lib/micro-ecc/types.h)

# Add result source
include_directories(lib/result)
set(RESULT_SOURCES
        ${CMAKE_SOURCE_DIR}/lib/result/result.h)

# Add AES-256 sources
include_directories(lib/AES)
set(AES_SOURCES
        ${CMAKE_SOURCE_DIR}/lib/AES/aes.h
        ${CMAKE_SOURCE_DIR}/lib/AES/aes.c)

# Include the schemes
add_subdirectory(schemes)

# Include project modules
set(PROTOMESH_MODULES communication cryptography)
foreach(MODULE ${PROTOMESH_MODULES})
    add_subdirectory(modules/${MODULE})
    include_directories(modules/${MODULE})
endforeach()


# Define targets
## Unit testing target
add_executable(unit_test ${PROTOMESH_TEST_FILES})
target_compile_definitions(unit_test PRIVATE UNIT_TESTING=1)
if (PROTOMESH_TEST_DEPS)
    add_dependencies(unit_test ${PROTOMESH_TEST_DEPS})
endif()