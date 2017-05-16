project(ucl_device_linux)

add_library(${PROJECT_NAME} linux.hpp linux.cpp)

find_package(Boost)
IF (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
endif ()
set(Boost_USE_STATIC_LIBS ON) # enable dynamic linking
set(Boost_USE_MULTITHREAD OFF)  # enable multithreading
find_package(Boost COMPONENTS REQUIRED system)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})