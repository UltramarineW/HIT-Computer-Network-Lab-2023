cmake_minimum_required(VERSION 3.22)

cmake_policy(SET CMP0048 NEW)

project(computer_network_lab)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-std=c++2a)

message(STATUS "CMAKE_SOURCE_DIR:${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(third_party/gflags)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/fmt)

add_subdirectory(lab1)
add_subdirectory(lab2)
