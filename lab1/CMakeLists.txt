project(lab1)

cmake_minimum_required(VERSION 3.1)

# find_package (gflags REQUIRED)
add_subdirectory(./third_party/gflags/)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

message(STATUS "CMAKE_SOURCE_DIR:${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

set (SRCS proxy_server.cpp)

add_executable(proxy_server main.cpp ${SRCS})

if (GFLAGS_USE_TARGET_NAMESPACE)
  if (TARGET gflags::gflags)
    target_link_libraries(proxy_server gflags::gflags)
  else ()
    message(FATAL_ERROR "gflags::gflags target not defined")
  endif ()
else ()
  if (TARGET gflags)
    target_link_libraries(proxy_server gflags)
  else ()
    message(FATAL_ERROR "gflags target not defined")
  endif ()
endif ()
