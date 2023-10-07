//
// Created by wujiayang on 2023/10/7.
//

#ifndef COMPUTER_NETWORK_LAB_PROXY_TASK_H
#define COMPUTER_NETWORK_LAB_PROXY_TASK_H

#include "winsock2.h"
#include "http_header_message.h"
#include "http_header_parser.h"
#include <string>

#define HTTP_PORT 80
#define HTTPS_PORT 443
#define MAXSIZE 65507

struct ProxyParam {
    SOCKET client_socket;
    SOCKET server_socket;
};

class ProxyTask {
public:

    explicit ProxyTask(ProxyParam *lpParameter);

    void Run();

private:

    static bool ConnectToServer(SOCKET *server_socket, char *host, int port);

    ProxyParam _proxy_parameter;
};


#endif //COMPUTER_NETWORK_LAB_PROXY_TASK_H
