//
// Created by wujiayang on 2023/10/7.
//

#include "proxy_task.h"

ProxyTask::ProxyTask(ProxyParam *lpParameter) : _proxy_parameter(*lpParameter) {

}


void ProxyTask::Run() {
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
        Sleep(200);
        closesocket(_proxy_parameter.client_socket);
        closesocket(_proxy_parameter.server_socket);
    }
    CacheBuffer = new char[recv_size + 1];
    ZeroMemory(CacheBuffer, recv_size + 1);
    memcpy(CacheBuffer, buffer, recv_size);
    parser.Parse(CacheBuffer);
    delete CacheBuffer;

    // send http message to server

    std::cout << "close socket" << std::endl;
    Sleep(200);
    closesocket(_proxy_parameter.client_socket);
    closesocket(_proxy_parameter.server_socket);
}