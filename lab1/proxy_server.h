#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <iostream>
#include <winsock2.h>
#include "http_message.h"
#include "http_header_parser.h"
#include "thread_pool.h"
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

    int server_port_;
    ThreadPool pool_;
    SOCKET server_socket_;
    sockaddr_in server_sockaddr_;
    std::shared_ptr<HttpFilter> filter_ptr_;
};

#endif