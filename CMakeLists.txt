cmake_minimum_required(VERSION 3.7)
project(openHome)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

set(FLATBUFFERS_DIR src/buffers)
include(cmake/BuildFlatbuffers.cmake)

set(DUKTAPE_SOURCES src/api/javascript/duktape/src/duk_config.h src/api/javascript/duktape/src/duktape.c src/api/javascript/duktape/src/duktape.h)
include_directories(src/api/javascript/duktape/src)

include_directories(lib/micro-ecc)

set(ECC_SOURCES
        lib/micro-ecc/uECC.h
        lib/micro-ecc/uECC.c
        lib/micro-ecc/uECC_vli.h
        lib/micro-ecc/types.h)
set(OPEN_HOME_SOURCE_FILES
        ${ECC_SOURCES}
        ${DUKTAPE_SOURCES}
        src/Network/NetworkManager.cpp
#        src/Network/NetworkManager.hpp
#        src/Network/Network.hpp
        src/Network/Network.cpp
#        src/const.hpp
#        src/api/time.hpp
#        src/api/network.hpp
        src/Device/Device.cpp
#        src/Device/Device.hpp
        src/Registry/RegistryEntry.cpp
#        src/Registry/RegistryEntry.hpp
#        src/crypto/crypto.hpp
#        src/crypto/sha512.hpp
        src/crypto/crypto.cpp
        src/crypto/crypto_asym.cpp
        src/Registry/Registry.cpp
#        src/Registry/Registry.hpp
#        src/api/storage.hpp
        src/api/javascript/javascript.cpp
#        src/api/javascript/javascript.hpp
        src/crypto/crypto_sym.cpp
        src/Network/Network.cpp
#        src/Network/Network.hpp
)
add_library(openHome ${OPEN_HOME_SOURCE_FILES})
add_dependencies(openHome flatbuffer_headers)

include_directories(lib/catch/single_include)
set(OPEN_HOME_TEST_FILES
        src/test/test.cpp
        lib/catch/single_include/catch.hpp
        ${OPEN_HOME_SOURCE_FILES})
add_executable(openHome_test ${OPEN_HOME_TEST_FILES})
target_compile_definitions(openHome_test PRIVATE UNIT_TESTING=1)
target_link_libraries(openHome_test flatbuffers)
add_dependencies(openHome_test flatbuffer_headers)

# Compilation of the linux specific example.
include_directories(src)
add_subdirectory(os-specifics/linux)
add_executable(openHome_debug examples/debug.cpp) # os-specifics/linux/linux.hpp
target_link_libraries(openHome_debug openHome openHome_device_linux)