cmake_minimum_required(VERSION 3.10)
project(irrlicht_test)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
find_package(Irrlicht)
include_directories(${IRRLICHT_INCLUDE_DIR})

add_executable(blackbox main.cpp)
target_link_libraries(blackbox ${IRRLICHT_LIBRARY})



