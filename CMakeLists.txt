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
set(PROTOMESH_TEST_FILES ${CMAKE_SOURCE_DIR}/modules/testing/test.cpp)
set(PROTOMESH_TEST_DEPS)

# Include project modules
add_subdirectory(modules/communication)


# Define targets
## Unit testing target
add_executable(unit_test ${PROTOMESH_TEST_FILES})
target_compile_definitions(unit_test PRIVATE UNIT_TESTING=1)
if (PROTOMESH_TEST_DEPS)
    add_dependencies(unit_test ${PROTOMESH_TEST_DEPS})
endif()