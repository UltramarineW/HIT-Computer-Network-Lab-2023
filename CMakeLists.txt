cmake_policy(SET CMP0048 NEW)

project(computer_network_lab)

cmake_minimum_required(VERSION 3.22)

message(STATUS "CMAKE_SOURCE_DIR:${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(third_party/gflags)

add_subdirectory(lab1)