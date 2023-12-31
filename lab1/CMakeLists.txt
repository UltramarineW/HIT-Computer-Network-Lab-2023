# set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set (SRCS proxy_server.cpp
        thread_pool.cpp
        http_header_parser.cpp
        proxy_task.cpp
        http_filter.cpp
        cache.cpp
        ../lab2/main.cpp)

add_executable(proxy_server main.cpp ${SRCS})

# add gflags support

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

# add winsock2 support
target_link_libraries(proxy_server wsock32 ws2_32)