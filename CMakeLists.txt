cmake_minimum_required(VERSION 3.7)
project(hoMesh)

# C++ compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Wno-unused-parameter")

# Add flatbuffers as compile target
set(FLATBUFFERS_DIR src/buffers)
include(cmake/BuildFlatbuffers.cmake)

# Add micro-ecc and its sources
include_directories(lib/micro-ecc)
set(ECC_SOURCES
        lib/micro-ecc/uECC.h
        lib/micro-ecc/uECC.c
        lib/micro-ecc/uECC_vli.h
        lib/micro-ecc/types.h)

# Add result source
include_directories(lib/result)
set(RESULT_SOURCES
        lib/result/result.h)

# Add AES-256 library
include_directories(lib/AES)
set(AES_SOURCES
        lib/AES/aes.h
        lib/AES/aes.c)

# Define the hoMesh source files and its dependency on ECC and Duktape
set(HoMesh_SOURCE_FILES
        ${AES_SOURCES}
        ${ECC_SOURCES}
        ${RESULT_SOURCES}
        src/const.hpp
        src/api/api.hpp
        src/api/time.hpp
        src/api/network.hpp
        src/api/storage.hpp
        src/api/keys.hpp
        src/crypto/sha512.hpp
        src/crypto/crypto.hpp
        src/crypto/crypto.cpp
        src/crypto/crypto_sym.cpp
        src/crypto/crypto_asym.cpp
        src/Logger/Logger.cpp
        src/Logger/Logger.hpp)

# Add the hoMesh library target and its dependency on the flatbuffers
add_library(hoMesh ${HoMesh_SOURCE_FILES})
add_dependencies(hoMesh flatbuffer_headers)


# Unit testing target
## Include the catch testing library
include_directories(lib/catch/single_include)
## Define the additional source files required for testing
set(HoMesh_TEST_FILES
        src/test/test.cpp
        ${HoMesh_SOURCE_FILES} src/api/network_test.cpp)
## Add the executable target and set the unit testing constant to true
add_executable(hoMesh_test ${HoMesh_TEST_FILES})
target_compile_definitions(hoMesh_test PRIVATE UNIT_TESTING=1)
## Link it against flatbuffers (for compile time serialization of json examples) and add the buffers for compilation
target_link_libraries(hoMesh_test flatbuffers)
add_dependencies(hoMesh_test flatbuffer_headers)


# Compilation of the linux specific example.
## Include hoMesh source files and the linux specific folder
include_directories(src)
add_subdirectory(os-specifics/linux)
## Add the executable target and link it to hoMesh and the linux library
add_executable(hoMesh_debug examples/debug.cpp)
target_link_libraries(hoMesh_debug hoMesh hoMesh_device_linux)
