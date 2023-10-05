project(lab1)

cmake_minimum_required(VERSION 3.1)

# find_package (gflags REQUIRED)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

message(STATUS "CMAKE_SOURCE_DIR:${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

set (SRCS proxy_server.cpp)

add_executable(proxy_server main.cpp ${SRCS})
