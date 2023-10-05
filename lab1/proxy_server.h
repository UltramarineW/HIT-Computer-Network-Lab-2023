#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <windows.h>

class ProxyServer {
public:
    ProxyServer(int port);
    void init();
};

#endif