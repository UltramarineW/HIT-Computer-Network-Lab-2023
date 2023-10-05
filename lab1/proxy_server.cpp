#include "proxy_server.h"

ProxyServer::ProxyServer(int port) {
    std::cout << "hello world from port: " << port << std::endl;
}

void ProxyServer::init() {
    std::cout << "init" << std::endl;
}