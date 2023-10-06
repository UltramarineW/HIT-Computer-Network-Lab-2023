#include <iostream>
#include "proxy_server.h"
#include "gflags/gflags.h"

DEFINE_int32(port, 10086, "Proxy Server Port");
DEFINE_string(listen_address, "127.0.0.1", "Proxy Server Listening Address");

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    ProxyServer server = ProxyServer(FLAGS_port);
    server.init();

    return 0;
}