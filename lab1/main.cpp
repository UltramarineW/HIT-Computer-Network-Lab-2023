#include <iostream>
#include "proxy_server.h"
#include "gflags/gflags.h"

DEFINE_int32(port, 10240, "Proxy Server Port");
DEFINE_string(listen_address, "127.0.0.1", "Proxy Server Listening Address");

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::cout << "[Info] Starting Proxy Server" << std::endl;
    ProxyServer server = ProxyServer(FLAGS_port);
    
    if (!server.InitSocket()) {
        std::cerr << "[Error] Server init socket failed" << std::endl;
    }

    server.Start();

    return 0;
}