//
// Created by wujiayang on 2023/10/7.
//

#include "proxy_task.h"

ProxyTask::ProxyTask(ProxyParam *lpParameter) : _proxy_parameter(*lpParameter) {

}

void ProxyTask::Run() {
    std::cout << "[Info] Run proxy task: client socket: " << _proxy_parameter.client_socket << std::endl;

    int recv_size;
    int ret;
    int length = sizeof(SOCKADDR_IN);
    char buffer[MAXSIZE];
    char *CacheBuffer;
    ZeroMemory(buffer, MAXSIZE);
    SOCKADDR_IN client_addr;
    HTTP_Header_Parser parser{};

    // receive http message from client
    recv_size = recv(_proxy_parameter.client_socket, buffer, MAXSIZE, 0);
    if (recv_size <= 0) {
        std::cerr << "[Error] Receive message size <= 0" << std::endl;
        Sleep(200);
        closesocket(_proxy_parameter.client_socket);
        return;
    }

    CacheBuffer = new char[recv_size + 1];
    ZeroMemory(CacheBuffer, recv_size + 1);
    memcpy(CacheBuffer, buffer, recv_size);
    parser.Parse(CacheBuffer);
    delete CacheBuffer;
    std::cout << parser.GetHeaderMessage() << std::endl;

    // connect to server
    if (!ConnectToServer(&_proxy_parameter.server_socket, parser.GetHeaderMessage().host, parser.GetHeaderMessage().port)) {
        std::cerr << "[Error] Connect to server failed" << std::endl;
        Sleep(200);
        closesocket(_proxy_parameter.client_socket);
        return;
    }
    std::cout << "[Info] Proxy server connected to host " << parser.GetHeaderMessage().host << " success" << std::endl;

    // send http message to server
    ret = send(_proxy_parameter.server_socket, buffer, strlen(buffer) + 1, 0);

    // receive server data
    recv_size = recv(_proxy_parameter.server_socket, buffer, MAXSIZE, 0);

    if (recv_size <= 0) {
        std::cerr << "[Error] Receive server host message size <= 0" << std::endl;
        Sleep(200);
        closesocket(_proxy_parameter.client_socket);
        closesocket(_proxy_parameter.server_socket);
        return;
    }

    std::cout << "return message:\n"  << buffer << std::endl;
    // resend message to client
    ret = send(_proxy_parameter.client_socket, buffer, sizeof(buffer), 0);

    std::cout << "[Info] Success. Proxy task end " << std::endl;
//    Sleep(200);
    closesocket(_proxy_parameter.client_socket);
    closesocket(_proxy_parameter.server_socket);
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
