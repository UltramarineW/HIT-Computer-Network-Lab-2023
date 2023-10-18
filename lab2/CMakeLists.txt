set (LAB2_SOURCE main.cpp
        stop_wait_client.cpp
        stop_wait_server.cpp
        transfer_massage.h
        utils.h
        utils.cpp)

find_package(Threads REQUIRED)

add_executable(file_transfer ${LAB2_SOURCE})

# add winsock2 support
target_link_libraries(file_transfer fmt::fmt spdlog::spdlog wsock32 ws2_32 gflags::gflags Threads::Threads)

