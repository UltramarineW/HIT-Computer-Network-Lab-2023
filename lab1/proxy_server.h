#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <winsock2.h>

#define HTTP_PORT 80
#define MAXSIZE 65507

struct HTTPHeader{
    char method[4];
    char url[1024];
    char host[1024];
    char cookie[1024*10];
    HTTPHeader () {
        ZeroMemory(this, sizeof(HTTPHeader));
    }
};

struct ProxyParam {
    SOCKET client_socket;
    SOCKET server_socket;
};


class ProxyServer {
public:
    ProxyServer(int port);
    // 服务器端初始化socket
    bool InitSocket();
    // 服务器端启动监听服务
    void Start();
    // 线程执行函数

private:
    static unsigned int __stdcall ProxyThread(LPVOID lpParameter);
    static void ParseHTTPHead(char* buffer, HTTPHeader* http_header);
    bool ConnectToServer(SOCKET* server_socket, char *host);

    int server_port;
    SOCKET server_socket;
    sockaddr_in server_sockaddr;
    ProxyParam* lp_proxy_param;
    HANDLE hThread;
};

#endif