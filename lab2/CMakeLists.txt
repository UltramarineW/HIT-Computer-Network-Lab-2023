set(LAB2_SOURCE
        SR_client.cpp
        SR_server.cpp
        transfer_message.h
        utils.h
        utils.cpp)

find_package(Threads REQUIRED)

# add winsock2 support
add_executable(file_transfer main.cpp ${LAB2_SOURCE})
target_link_libraries(file_transfer fmt::fmt spdlog::spdlog wsock32 ws2_32 gflags::gflags Threads::Threads)

#add_executable(sr_server server_main.cpp ${LAB2_SOURCE})
#add_executable(sr_client client_main.cpp ${LAB2_SOURCE})
#target_link_libraries(sr_server fmt::fmt spdlog::spdlog wsock32 ws2_32 gflags::gflags Threads::Threads)
#target_link_libraries(sr_client fmt::fmt spdlog::spdlog wsock32 ws2_32 gflags::gflags Threads::Threads)


