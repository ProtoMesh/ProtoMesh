cmake_minimum_required(VERSION 3.7)
project(openHome)

# C++ compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

# Add flatbuffers as compile target
set(FLATBUFFERS_DIR src/buffers)
include(cmake/BuildFlatbuffers.cmake)

# Add duktape sources
set(DUKTAPE_SOURCES src/api/javascript/duktape/src/duk_config.h src/api/javascript/duktape/src/duktape.c src/api/javascript/duktape/src/duktape.h)
include_directories(src/api/javascript/duktape/src)

# Add micro-ecc and its sources
include_directories(lib/micro-ecc)
set(ECC_SOURCES
        lib/micro-ecc/uECC.h
        lib/micro-ecc/uECC.c
        lib/micro-ecc/uECC_vli.h
        lib/micro-ecc/types.h)

# Add uint128_t sources
include_directories(lib/uint128_t)
set(UINT128_SOURCES
        lib/uint128_t/uint128_t.cpp)

# Define the openHome source files and its dependency on ECC and Duktape
set(OPEN_HOME_SOURCE_FILES
        ${ECC_SOURCES}
        ${DUKTAPE_SOURCES}
        ${UINT128_SOURCES}
        src/Network/NetworkManager.cpp
        src/Network/NetworkManager.hpp
        src/Network/Network.hpp
        src/Network/Network.cpp
        src/const.hpp
        src/api/time.hpp
        src/api/network.hpp
        src/Device/Device.cpp
        src/Device/Device.hpp
        src/Registry/RegistryEntry.cpp
        src/Registry/RegistryEntry.hpp
        src/crypto/crypto.hpp
        src/crypto/sha512.hpp
        src/crypto/crypto.cpp
        src/crypto/crypto_asym.cpp
        src/Registry/Registry.cpp
        src/Registry/Registry.hpp
        src/api/storage.hpp
        src/api/javascript/javascript.cpp
        src/api/javascript/javascript.hpp
        src/crypto/crypto_sym.cpp
        src/Network/Network.cpp
        src/Network/Network.hpp
)
# Add the openHome library target and its dependency on the flatbuffers
add_library(openHome ${OPEN_HOME_SOURCE_FILES})
add_dependencies(openHome flatbuffer_headers)


# Unit testing target
## Include the catch testing library
include_directories(lib/catch/single_include)
## Define the additional source files required for testing
set(OPEN_HOME_TEST_FILES
        src/test/test.cpp
        ${OPEN_HOME_SOURCE_FILES} src/api/network_test.cpp)
## Add the executable target and set the unit testing constant to true
add_executable(openHome_test ${OPEN_HOME_TEST_FILES})
target_compile_definitions(openHome_test PRIVATE UNIT_TESTING=1)
## Link it against flatbuffers (for compile time serialization of json examples) and add the buffers for compilation
target_link_libraries(openHome_test flatbuffers)
add_dependencies(openHome_test flatbuffer_headers)


# Compilation of the linux specific example.
## Include openHome source files and the linux specific folder
include_directories(src)
add_subdirectory(os-specifics/linux)
## Add the executable target and link it to openHome and the linux library
add_executable(openHome_debug examples/debug.cpp)
target_link_libraries(openHome_debug openHome openHome_device_linux)