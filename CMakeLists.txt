cmake_minimum_required(VERSION 3.7)
project(openHome)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

set(FLATBUFFERS_DIR deviceLib/buffers)
include(cmake/BuildFlatbuffers.cmake)

add_subdirectory(osLib/linux)
include_directories(deviceLib/test/catch/single_include)

set(DUKTAPE_SOURCES deviceLib/api/javascript/duktape/src/duk_config.h deviceLib/api/javascript/duktape/src/duktape.c deviceLib/api/javascript/duktape/src/duktape.h)
include_directories(deviceLib/api/javascript/duktape/src)

set(ECC_SOURCES deviceLib/crypto/ecc/uECC.h deviceLib/crypto/ecc/uECC.c deviceLib/crypto/ecc/uECC_vli.h deviceLib/crypto/ecc/types.h)
set(DEVICE_LIB_SOURCE_FILES ${ECC_SOURCES} ${DUKTAPE_SOURCES} deviceLib/Network/NetworkManager.cpp deviceLib/Network/NetworkManager.hpp deviceLib/Network/Network.hpp deviceLib/Network/Network.cpp deviceLib/const.hpp deviceLib/api/time.hpp deviceLib/json/ArduinoJson.hpp deviceLib/api/network.hpp deviceLib/Device/Device.cpp deviceLib/Device/Device.hpp deviceLib/Registry/RegistryEntry.cpp deviceLib/Registry/RegistryEntry.hpp deviceLib/crypto/crypto.hpp deviceLib/crypto/sha512.hpp deviceLib/crypto/crypto.cpp deviceLib/crypto/crypto_asym.cpp deviceLib/Registry/Registry.cpp deviceLib/Registry/Registry.hpp deviceLib/api/storage.hpp deviceLib/api/javascript/javascript.cpp deviceLib/api/javascript/javascript.hpp deviceLib/crypto/crypto_sym.cpp)
add_library(openHome ${DEVICE_LIB_SOURCE_FILES} deviceLib/Network/Network.cpp deviceLib/Network/Network.hpp)
add_dependencies(openHome flatbuffer_headers)


add_executable(openHome_test deviceLib/test/test.cpp deviceLib/test/catch/single_include/catch.hpp ${DEVICE_LIB_SOURCE_FILES})
target_compile_definitions(openHome_test PRIVATE UNIT_TESTING=1)
target_link_libraries(openHome_test flatbuffers)
add_dependencies(openHome_test flatbuffer_headers)


add_executable(openHome_debug examples/debug.cpp osLib/linux/linux.hpp)
target_link_libraries(openHome_debug openHome openHome_device_linux)