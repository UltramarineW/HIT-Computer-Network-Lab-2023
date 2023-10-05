#include <iostream>
#include "proxy_server.h"
#include "gflags/gflags.h"

DEFINE_int32(port, 10086, "Proxy Server Port");

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    ProxyServer server = ProxyServer(FLAGS_port);
    server.init();
    return 0;
}