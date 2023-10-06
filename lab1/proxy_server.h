#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <windows.h>

#define HTTP_PORT 80
#define MAX_LENGTH 65507

typedef struct HTTPHeader{
    char method[4];
    char url[1024];
    char host[1024];
    char cookie[1024*10];
} HTTPHeader;


class ProxyServer {
public:
    ProxyServer(int port);
    void init();
};

#endif