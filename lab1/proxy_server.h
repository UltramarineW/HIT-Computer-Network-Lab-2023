#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <winsock2.h>
#include "http_header_message.h"
#include "http_header_parser.h"
#include "ThreadPool.h"
#include "proxy_task.h"
#include "gflags/gflags.h"

DECLARE_int32(thread_nums);




class ProxyServer {
public:
    ProxyServer(int port);
    // 服务器端启动
    void Start();

private:
    // 服务器端初始化socket
    bool InitSocket();
    // 线程执行函数
    static unsigned int __stdcall ProxyThread(LPVOID lpParameter);

    // 连接服务器端
    bool ConnectToServer(SOCKET* server_socket, char *host);

    int server_port;
    // TODO: 多线程支持
    ThreadPool pool;
    SOCKET server_socket;
    sockaddr_in server_sockaddr;
    ProxyParam* lp_proxy_param;
    HANDLE hThread;
};

#endif