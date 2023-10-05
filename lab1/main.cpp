#include <iostream>
#include "proxy_server.h"

int main(int argc, char *argv[]) {
    ProxyServer server = ProxyServer(10);
    server.init();
    return 0;
}