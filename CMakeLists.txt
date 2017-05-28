cmake_minimum_required(VERSION 3.7)
project(ucl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

add_subdirectory(osLib/linux)
include_directories(deviceLib/test/catch/single_include)

set(ECC_SOURCES deviceLib/crypto/ecc/uECC.h deviceLib/crypto/ecc/uECC.c deviceLib/crypto/ecc/uECC_vli.h deviceLib/crypto/ecc/types.h)
set(DEVICE_LIB_SOURCE_FILES ${ECC_SOURCES} deviceLib/json/ArduinoJson.hpp deviceLib/api/network.hpp deviceLib/Device.cpp deviceLib/Device.hpp deviceLib/registry/RegistryEntry.cpp deviceLib/registry/RegistryEntry.hpp deviceLib/crypto/crypto.hpp deviceLib/crypto/sha512.hpp deviceLib/crypto/crypto.cpp deviceLib/crypto/crypto_asym.cpp deviceLib/registry/Registry.cpp deviceLib/registry/Registry.hpp deviceLib/api/storage.hpp)
add_library(ucl ${DEVICE_LIB_SOURCE_FILES} deviceLib/const.hpp deviceLib/api/time.hpp)

add_executable(ucl_test deviceLib/test/test.cpp deviceLib/test/catch/single_include/catch.hpp ${DEVICE_LIB_SOURCE_FILES})
target_compile_definitions(ucl_test PRIVATE UNIT_TESTING=1)

add_executable(ucl_debug examples/debug.cpp osLib/linux/linux.hpp)
target_link_libraries(ucl_debug ucl ucl_device_linux)