//
// Created by wujiayang on 2023/10/7.
//

#include "proxy_task.h"

#include <utility>

ProxyTask::ProxyTask(ProxyParam lpParameter, std::shared_ptr<HttpFilter> filter_ptr) : proxy_parameter_(lpParameter), parser_ptr_(std::make_unique<HttpHeaderParser>()), filter_ptr_(filter_ptr), add_modify_(false){}

void ProxyTask::Run() {
    std::cout << "[Info] Run proxy task: client socket: " << proxy_parameter_.client_socket << std::endl;

    int recv_size;
    int ret;
    int length = sizeof(SOCKADDR_IN);
    char buffer[MAXSIZE];
    char *CacheBuffer;
    ZeroMemory(buffer, MAXSIZE);
    SOCKADDR_IN client_addr;
    DWORD wait_time = 0;

    // receive http request from client
    recv_size = recv(proxy_parameter_.client_socket, buffer, MAXSIZE, 0);
    std::cout << "[Info] Receive request message from client" << std::endl;
    if (recv_size <= 0) {
        std::cerr << "[Error] Receive message size <= 0" << std::endl;
        CloseSocketAndWait(wait_time);
        return;
    }

    CacheBuffer = new char[recv_size + 1];
    ZeroMemory(CacheBuffer, recv_size + 1);
    memcpy(CacheBuffer, buffer, recv_size);
    parser_ptr_->ParseRequest(CacheBuffer);
    delete CacheBuffer;
    // request message

    // filter https message
    if (std::string(parser_ptr_->GetHeaderMessage().method) == "CONNECT") {
        std::cout << "[Info] HTTPS message, pass" << std::endl;
        CloseSocketAndWait(wait_time);
        return;
    }

    // filter http message
    if (!filter_ptr_->Filter(parser_ptr_->GetHeaderMessage())) {
        std::cout << "[Info] Block a request from " << parser_ptr_->GetHeaderMessage().host << std::endl;
        CloseSocketAndWait(wait_time);
        return;
    }

    AddHeaderCacheSegment(parser_ptr_->GetHeaderMessage(), buffer);

    // connect to server
    if (!ConnectToServer(&proxy_parameter_.server_socket, parser_ptr_->GetHeaderMessage().host,
                         parser_ptr_->GetHeaderMessage().port)) {
        std::cerr << "[Error] Connect to server failed" << std::endl;
        CloseSocketAndWait(wait_time);
        return;
    }
    std::cout << "[Info] Proxy server connected to host " << parser_ptr_->GetHeaderMessage().host << " success" << std::endl;

    // send http message to server
    ret = send(proxy_parameter_.server_socket, buffer, strlen(buffer) + 1, 0);

    std::cout << "[Info] Send message to " << parser_ptr_->GetHeaderMessage().host << std::endl;

    // receive server data
    recv_size = recv(proxy_parameter_.server_socket, buffer, MAXSIZE, 0);

    std::cout << "[Info] Receive message from " << parser_ptr_->GetHeaderMessage().host << std::endl;

    if (recv_size <= 0) {
        std::cerr << "[Error] Receive server host message size <= 0" << std::endl;
        CloseSocketAndWait(wait_time);
        return;
    }

     ProcessAndCacheResponse(buffer, recv_size, parser_ptr_->GetHeaderMessage());

    // resend message to client
//    std::cout << buffer << std::endl;
    ret = send(proxy_parameter_.client_socket, buffer, sizeof(buffer), 0);

    std::cout << "[Info] Resend response message to client" << std::endl;

    CloseSocketAndWait(0);
    std::cout << "[Info] Success, Proxy task end, close client socket: " << proxy_parameter_.client_socket << " server socket: " << proxy_parameter_.server_socket << std::endl;
}

bool ProxyTask::ConnectToServer(SOCKET *server_socket, char *host, int port) {
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    char *url = strtok(host, ":");
    HOSTENT *hostent = gethostbyname(url);
    if (!hostent) {
        return false;
    }
    in_addr in_addr_host = *((in_addr *) *hostent->h_addr_list);
    server_addr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(in_addr_host));
    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket == INVALID_SOCKET) {
        return false;
    }
    if (connect(*server_socket, (SOCKADDR *) &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(*server_socket);
        return false;
    }
    return true;
}

void ProxyTask::CloseSocketAndWait(DWORD wait_time) const {
    closesocket(proxy_parameter_.client_socket);
    closesocket(proxy_parameter_.client_socket);
    Sleep(wait_time);
}

void ProxyTask::AddHeaderCacheSegment(HttpHeader header, char* buffer) {
    Cache* cache = Cache::GetInstance();
    CacheInstance *instance = cache->Find(header.host, header.url);
    char *if_modified_since_str = "\r\nIf-Modified-Since: ";
    char *modified_time = instance->modified_gmt;
    // find instance in cache
    if (instance != nullptr && header.if_modified_since[0] == '\0') {
        strcat(buffer, if_modified_since_str);
        strcat(buffer, modified_time);
        add_modify_ = true;
        std::cout << "[Info] Add If-Modified-Since segment" << std::endl;
    }
}

void ProxyTask::ProcessAndCacheResponse(char *buffer, size_t recv_size, const HttpHeader& request_header) const {
    char *cache_buffer = new char[recv_size+1];
    ZeroMemory(cache_buffer, recv_size+1);
    memcpy(cache_buffer, buffer, recv_size);

    // continue to process http response message
    parser_ptr_->ParseResponse(cache_buffer);

    if (parser_ptr_->GetHeaderMessage().state_word == 200
        && parser_ptr_->GetHttpMessage().body != nullptr) {
        auto cache_instance = Cache::GetInstance();
        std::cout << "[Info] Add cache line" << std::endl;
        cache_instance->Add(parser_ptr_->GetHttpMessage());
    }

}