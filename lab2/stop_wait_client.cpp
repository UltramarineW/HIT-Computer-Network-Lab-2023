//
// Created by wujiayang on 2023/10/17.
//

#include "stop_wait_client.h"

#include <utility>

StopWaitClient::StopWaitClient(const unsigned int& port,std::string  ip) : port_(port), ip_(std::move(ip)){
    spdlog::info("udp client start");
}

int StopWaitClient::Start() {
    // create client socket
    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == INVALID_SOCKET) {
        spdlog::error("socket create error, error code: {}", WSAGetLastError());
        return -1;
    } else {
        spdlog::info("client socket create success: {}", client_socket);
    }

    // setup address structure
    SOCKADDR_IN addr_server;
    addr_server.sin_addr.S_un.S_addr = inet_addr(ip_.c_str());
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port_);


    // set client recv timeout
    int client_recv_timeout = 2 * 1000; // 2s
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&client_recv_timeout, sizeof(client_recv_timeout)) == -1) {
        spdlog::error("set receive timeout fail");
    }

    int random_seq = GetRandomInteger(SEQ_MIN, SEQ_MAX);

    TransferMassage client_transfer_message{0, 0, "hello from client"};

    // start communication
    while(true) {
        // send message
        char buffer[BUFFER_LENGTH];
        ZeroMemory(buffer, BUFFER_LENGTH);

        strcpy(buffer, MessageToString(client_transfer_message).c_str());
        if (sendto(client_socket, buffer, strlen(buffer), 0, (sockaddr*)&addr_server, sizeof(sockaddr_in)) == SOCKET_ERROR) {
            spdlog::error("client send message fail, error code: {}", WSAGetLastError());
            return -1;
        }

        // recv answer
        ZeroMemory(buffer, BUFFER_LENGTH);
        int slen = sizeof(sockaddr_in);
        int answer_length;
        answer_length = recvfrom(client_socket, buffer, BUFFER_LENGTH, 0, (sockaddr*)&addr_server, &slen);
        if (answer_length == SOCKET_ERROR) {
            spdlog::warn("client recv from server failed, error code: {}", WSAGetLastError());
            continue;
        } else {
            spdlog::debug("client receive message from {}:{}", inet_ntoa(addr_server.sin_addr), htons(addr_server.sin_port));
        }

        TransferMassage server_message;
        if (StringToMessage(std::string(buffer), server_message) == -1) {
            spdlog::error("parse client message string failed");
            return -1;
        } else {
            spdlog::info("server->client: seq: {} ack: {} data: {}", server_message.seq, server_message.ack, server_message.data);
        }

        if( ProcessServerMessage(server_message) < 0) {
            spdlog::error("client process server message fail");
            return -1;
        } else {
            spdlog::debug("process server message success");
        }
        client_transfer_message = std::move(server_message);

        Sleep(1000);
    }

    closesocket(client_socket);
    spdlog::info("close client socket");
    return 0;
}


int StopWaitClient::ProcessServerMessage(TransferMassage &message) const {
    message.ack = (message.seq + 1);
    message.data = "hello from client";
    return 0;
}