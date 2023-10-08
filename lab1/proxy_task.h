//
// Created by wujiayang on 2023/10/7.
//

#ifndef COMPUTER_NETWORK_LAB_PROXY_TASK_H
#define COMPUTER_NETWORK_LAB_PROXY_TASK_H

#include <string>
#include "winsock2.h"
#include "gflags/gflags.h"
#include "http_header_message.h"
#include "http_header_parser.h"
#include "http_filter.h"

#define HTTP_PORT 80
#define HTTPS_PORT 443
#define MAXSIZE 65507


struct ProxyParam {
    SOCKET client_socket;
    SOCKET server_socket;
};

class ProxyTask {
public:

    explicit ProxyTask(ProxyParam lpParameter, std::shared_ptr<HTTPFilter> filter_ptr);

    void Run();

private:

    static bool ConnectToServer(SOCKET *server_socket, char *host, int port);
    static bool WebSiteFilter(HTTPHeader* http_header);

    ProxyParam _proxy_parameter;
    std::unique_ptr<HTTP_Header_Parser> _parser_ptr;
    std::shared_ptr<HTTPFilter> _filter_ptr;
};


#endif //COMPUTER_NETWORK_LAB_PROXY_TASK_H
