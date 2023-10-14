#include <iostream>
#include "proxy_server.h"
#include "gflags/gflags.h"

DEFINE_int32(port, 10240, "Proxy Server Port");
DEFINE_string(listen_address, "127.0.0.1", "Proxy Server Listening Address");
DEFINE_int32(thread_nums, 12, "Thread numbers in thread pool_");
DEFINE_int32(website_filter_protocol, 0, "Proxy server website filter protocol: "
                                         " 0 for no filter protocol"
                                         " 1 for white list filter"
                                         " 2 for black list filter");
DEFINE_int32(fishing_function, 0, "Whether start fishing function:"
                                  " 0 for off"
                                  " 1 for on");
DEFINE_int32(user_filter_protocol, 0, "Proxy server user filter protocol:"
                                      " 0 for no filter protocol"
                                      " 1 for white list filter"
                                      " 2 for black list filter");
DEFINE_bool(use_cache, false, "Whether to use cache method:"
                          " false for no cache"
                          " true for use cache");

int main(int argc, char *argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    std::cout << "[Info] Starting Proxy Server" << std::endl;
    ProxyServer server = ProxyServer(FLAGS_port);

    server.Start();

    return 0;
}