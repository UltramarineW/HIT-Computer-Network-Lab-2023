//
// Created by wujiayang on 2023/10/7.
//

#ifndef COMPUTER_NETWORK_LAB_PROXY_TASK_H
#define COMPUTER_NETWORK_LAB_PROXY_TASK_H

#include <string>
#include "winsock2.h"
#include "gflags/gflags.h"
#include "http_message.h"
#include "http_header_parser.h"
#include "http_filter.h"
#include "cache.h"
#include "gflags/gflags.h"

#define HTTP_PORT 80
#define HTTPS_PORT 443
#define MAXSIZE 1000000

DECLARE_int32(fishing_function);
DECLARE_bool(use_cache);

struct ProxyParam {
    SOCKET client_socket;
    SOCKET server_socket;
};

class ProxyTask {
public:

    explicit ProxyTask(ProxyParam lpParameter, std::shared_ptr<HttpFilter> filter_ptr);

    ~ProxyTask() {delete[] buffer;}

    void Run();

private:

    static bool ConnectToServer(SOCKET *server_socket, char *host, int port);
    void CloseSocketAndWait(DWORD wait_time) const;
    void AddHeaderCacheSegment(HttpHeader header, char* buffer);
    void ProcessAndCacheResponse(char* buffer, size_t recv_size,const HttpHeader& request_header) const;

    ProxyParam proxy_parameter_;
    std::unique_ptr<HttpHeaderParser> parser_ptr_;
    std::shared_ptr<HttpFilter> filter_ptr_;
    bool add_modify_;
    char *buffer;
};


#endif //COMPUTER_NETWORK_LAB_PROXY_TASK_H
