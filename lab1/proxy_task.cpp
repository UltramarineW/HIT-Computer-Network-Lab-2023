//
// Created by wujiayang on 2023/10/7.
//

#include "proxy_task.h"

#include <utility>
#include <sstream>

ProxyTask::ProxyTask(ProxyParam lpParameter, std::shared_ptr<HttpFilter> filter_ptr) : proxy_parameter_(lpParameter),
                                                                                       parser_ptr_(
                                                                                               std::make_unique<HttpHeaderParser>()),
                                                                                       filter_ptr_(filter_ptr),
                                                                                       add_modify_(false) {}

void ProxyTask::Run() {
    std::cout << "[Info] Run proxy task: client socket: " << proxy_parameter_.client_socket << std::endl;

//    char buffer[MAXSIZE];
    buffer = new char[MAXSIZE];
    int recv_size;
    int ret;
    int length = sizeof(SOCKADDR_IN);
//    char buffer[MAXSIZE];
//    char *buffer = new char[MAXSIZE];
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
    if (FLAGS_fishing_function) {
        char* new_buffer_for_fishing = "GET http://www.news.cn/politics/leaders/2023-04/03/c_1129491534.htm HTTP/1.1\r\nHost: www.news.cn\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/118.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\r\nAccept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2\r\nAccept-Encoding: gzip, deflate\r\nReferer: http://news.hit.edu.cn/\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n\r\n";
        ZeroMemory(buffer, MAXSIZE);
        memcpy(buffer, new_buffer_for_fishing, strlen(new_buffer_for_fishing)+1);
        recv_size = 529;
    }

    CacheBuffer = new char[recv_size + 1];
    ZeroMemory(CacheBuffer, recv_size + 1);
    memcpy(CacheBuffer, buffer, recv_size);
    parser_ptr_->ParseRequest(CacheBuffer);
    delete CacheBuffer;

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

    if (FLAGS_use_cache) {
        AddHeaderCacheSegment(parser_ptr_->GetHeaderMessage(), buffer);
    }

    // connect to server
    if (!ConnectToServer(&proxy_parameter_.server_socket, parser_ptr_->GetHeaderMessage().host,
                         parser_ptr_->GetHeaderMessage().port)) {
        std::cerr << "[Error] Connect to server failed" << std::endl;
        CloseSocketAndWait(wait_time);
        return;
    }
    std::cout << "[Info] Proxy server connected to host " << parser_ptr_->GetHeaderMessage().host << " success"
              << std::endl;

    // send http message to server
    ret = send(proxy_parameter_.server_socket, buffer, recv_size, 0);

    std::cout << "[Info] Send message to " << parser_ptr_->GetHeaderMessage().host << std::endl;

    ZeroMemory(buffer, MAXSIZE);


    // receive server data
    recv_size = recv(proxy_parameter_.server_socket, buffer, MAXSIZE, 0);

    std::cout << "[Info] Receive message from " << parser_ptr_->GetHeaderMessage().host << std::endl;

    if (recv_size <= 0) {
        std::cerr << "[Error] Receive server host message size <= 0" << std::endl;
        CloseSocketAndWait(wait_time);
        return;
    }

    if (FLAGS_use_cache) {
        ProcessAndCacheResponse(buffer, recv_size, parser_ptr_->GetHeaderMessage());
    }

    // resend message to client

    ret = send(proxy_parameter_.client_socket, buffer, recv_size+1, 0);


    std::cout << "[Info] Resend response message to client" << std::endl;

    CloseSocketAndWait(200);
    std::cout << "[Info] Success, Proxy task end, close client socket: " << proxy_parameter_.client_socket
              << " server socket: " << proxy_parameter_.server_socket << std::endl;
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

void ProxyTask::AddHeaderCacheSegment(HttpHeader header, char *buffer) {
    Cache *cache = Cache::GetInstance();
    CacheInstance *instance = cache->Find(header.host, header.url);

    if (instance == nullptr)
        return;

    char *if_modified_since_str = "If-Modified-Since: ";
    // find instance in cache
    if (instance != nullptr && header.if_modified_since[0] == '\0') {
        // 去除末尾\r\n
        buffer[strlen(buffer)-2] = '\0';
        strcat(buffer, if_modified_since_str);
        strcat(buffer, instance->modified_gmt.c_str());
        strcat(buffer, "\r\n\r\n");
        add_modify_ = true;
        std::cout << "[Info] Add If-Modified-Since segment" << std::endl;
    }
}

void ProxyTask::ProcessAndCacheResponse(char *buffer, size_t recv_size, const HttpHeader &request_header) const {
    char *cache_buffer = new char[recv_size + 1];
    ZeroMemory(cache_buffer, recv_size + 1);
    memcpy(cache_buffer, buffer, recv_size);

    // continue to process http response message
    parser_ptr_->ParseResponse(cache_buffer);

    if (parser_ptr_->GetHeaderMessage().state_word == 200
        && parser_ptr_->GetHttpMessage().body != nullptr) {
        auto cache_instance = Cache::GetInstance();
        std::cout << "[Info] Add cache line" << std::endl;
        cache_instance->Add(parser_ptr_->GetHttpMessage(), buffer);
    }

    else if (parser_ptr_->GetHeaderMessage().state_word == 304) {
        auto cache_instance = Cache::GetInstance();
        std::cout << "[Info] Find cache line" << std::endl;
        auto instance = cache_instance->Find(parser_ptr_->GetHeaderMessage().host,
                             parser_ptr_->GetHeaderMessage().url);
        char filename[100];
        snprintf(filename, sizeof(filename), "E:/HIT_Project/HIT-Computer-Network-Lab-2023/lab1/cache/%zu", instance->id); // 保存在cache目录下
        std::ifstream cache_file(std::string(filename), std::ios::in);
        std::stringstream cache_content;
        cache_content << cache_file.rdbuf();
        ZeroMemory(buffer, MAXSIZE);
        memcpy(buffer, cache_content.str().c_str(), cache_content.str().length());
    }

}