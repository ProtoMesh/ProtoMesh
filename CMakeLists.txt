cmake_minimum_required(VERSION 3.7)
project(ucl)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(osLib/linux)

set(ECC_SOURCES deviceLib/crypto/ecc/uECC.h deviceLib/crypto/ecc/uECC.c deviceLib/crypto/ecc/uECC_vli.h deviceLib/crypto/ecc/types.h)
set(DEVICE_LIB_SOURCE_FILES ${ECC_SOURCES} deviceLib/json/ArduinoJson.hpp deviceLib/network.h deviceLib/Device.cpp deviceLib/Device.hpp deviceLib/Registry.cpp deviceLib/Registry.hpp deviceLib/crypto/crypto.hpp deviceLib/crypto/sha512.hpp)
add_library(ucl ${DEVICE_LIB_SOURCE_FILES})

add_executable(ucl_debug examples/debug.cpp osLib/linux/linux.hpp)
target_link_libraries(ucl_debug ucl ucl_device_linux)