//
// Created by wujiayang on 2023/10/22.
//
#include <iostream>
#include <gflags/gflags.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <winsock2.h>
#include <thread>

#include "SR_client.h"
#include "utils.h"

DEFINE_uint32(port, 12400, "port for server to listen on and for client to bind");
DEFINE_string(ip, "127.0.0.1", "IP address to listen on");
DEFINE_bool(debug, false, "debug mode");

int LoadSocketLibrary() {
    // load socket library
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        spdlog::error("WSAStartup failed with error {}", err);
        return -1;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        spdlog::error("Can't find a usable version of winsock.dll");
        WSACleanup();
        return -1;
    } else {
        spdlog::info("winsock2.dll found and initialized");
    }
    return 0;
}

int main(int argc, char *argv[]) {
    // Initialize third party libraries
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    if (FLAGS_debug) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }
    spdlog::info("File transfer application start");
    // Initialize socket
    if (LoadSocketLibrary() < 0) {
        exit(-1);
    }

    // Create Server and Client
    SRClient client(FLAGS_port, FLAGS_ip);


    std::thread client_thread([&client]() {
        int err = client.Start();
        if (err < 0) {
            spdlog::error("client return error");
            exit(err);
        }
    });
    client_thread.join();

    return 0;
}

