cmake_minimum_required(VERSION 3.7)
project(lumos)

# C++ compiler settings
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Wno-unused-parameter")

# Add flatbuffers as compile target
set(FLATBUFFERS_DIR src/buffers)
include(cmake/BuildFlatbuffers.cmake)

# Add duktape sources
set(DUKTAPE_SOURCES lib/duktape/src/duk_config.h lib/duktape/src/duktape.c lib/duktape/src/duktape.h)
include_directories(lib/duktape/src)

# Add micro-ecc and its sources
include_directories(lib/micro-ecc)
set(ECC_SOURCES
        lib/micro-ecc/uECC.h
        lib/micro-ecc/uECC.c
        lib/micro-ecc/uECC_vli.h
        lib/micro-ecc/types.h)

# Define the lumos source files and its dependency on ECC and Duktape
set(LUMOS_SOURCE_FILES
        ${ECC_SOURCES}
        ${DUKTAPE_SOURCES}
        src/Network/NetworkManager.cpp
        src/Network/NetworkManager.hpp
        src/Network/Network.hpp
        src/Network/Network.cpp
        src/const.hpp
        src/api/api.hpp
        src/api/time.hpp
        src/api/network.hpp
        src/api/storage.hpp
        src/api/keys.hpp
        src/Device/Device.cpp
        src/Device/Device.hpp
        src/crypto/sha512.hpp
        src/crypto/crypto.cpp
        src/crypto/crypto_asym.cpp
        src/Registry/Registry.hpp
        src/Registry/Registry.cpp
        src/Registry/RegistrySync.cpp
        src/Registry/RegistryAddition.cpp
        src/Registry/Entry/RegistryEntry.cpp
        src/Registry/Entry/RegistryEntry.hpp
        src/crypto/crypto.hpp
        src/crypto/crypto_sym.cpp
        src/Network/Network.cpp
        src/Network/Network.hpp
        src/Registry/Validators.hpp)
# Add the lumos library target and its dependency on the flatbuffers
add_library(lumos ${LUMOS_SOURCE_FILES})
add_dependencies(lumos flatbuffer_headers)


# Unit testing target
## Include the catch testing library
include_directories(lib/catch/single_include)
## Define the additional source files required for testing
set(LUMOS_TEST_FILES
        src/test/test.cpp
        ${LUMOS_SOURCE_FILES} src/api/network_test.cpp)
## Add the executable target and set the unit testing constant to true
add_executable(lumos_test ${LUMOS_TEST_FILES})
target_compile_definitions(lumos_test PRIVATE UNIT_TESTING=1)
## Link it against flatbuffers (for compile time serialization of json examples) and add the buffers for compilation
target_link_libraries(lumos_test flatbuffers)
add_dependencies(lumos_test flatbuffer_headers)


# Compilation of the linux specific example.
## Include lumos source files and the linux specific folder
include_directories(src)
add_subdirectory(os-specifics/linux)
## Add the executable target and link it to lumos and the linux library
add_executable(lumos_debug examples/debug.cpp)
target_link_libraries(lumos_debug lumos lumos_device_linux)
