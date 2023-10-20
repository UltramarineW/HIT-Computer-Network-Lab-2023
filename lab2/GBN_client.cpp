//
// Created by wujiayang on 2023/10/17.
//

#include "GBN_client.h"

#include <utility>

GBNClient::GBNClient(const unsigned int& port, std::string  ip) : port_(port), ip_(std::move(ip)), base_(0){
    spdlog::debug("udp client start");
}


int GBNClient::InitClientServerSocket() {
    // create client socket
    client_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket_ == INVALID_SOCKET) {
        spdlog::error("socket create error, error code: {}", WSAGetLastError());
        return -1;
    } else {
        spdlog::info("client socket create success: {}", client_socket_);
    }
    // setup address structure
    addr_server_.sin_addr.S_un.S_addr = inet_addr(ip_.c_str());
    addr_server_.sin_family = AF_INET;
    addr_server_.sin_port = htons(port_);

    // set client recv timeout
    int client_recv_timeout = 3 * 1000; // 2s
    if (setsockopt(client_socket_, SOL_SOCKET, SO_RCVTIMEO, (char*)&client_recv_timeout, sizeof(client_recv_timeout)) == -1) {
        spdlog::error("set receive timeout fail");
    }
    return 0;
}

int GBNClient::HandshakeProcess() {
    TransferMessage client_handshake_message{0, 0, "hello from client"};

    // send message
    char buffer[BUFFER_LENGTH];
    ZeroMemory(buffer, BUFFER_LENGTH);

    strcpy(buffer, MessageToString(client_handshake_message).c_str());
    if (sendto(client_socket_, buffer, strlen(buffer), 0, (sockaddr*)&addr_server_, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        spdlog::error("client send message fail, error code: {}", WSAGetLastError());
        return -1;
    }
    return 0;
}

int GBNClient::Start() {

    if (InitClientServerSocket() < 0) {
        return -1;
    }

    if (HandshakeProcess() < 0) {
        spdlog::error("client handshake process error");
        return -1;
    } else {
        spdlog::debug("client handshake process success");
    }

    std::string client_transfer_message;
    char buffer[BUFFER_LENGTH];

    // start communication
    while(true) {
        // recv answer
        ZeroMemory(buffer, BUFFER_LENGTH);
        int slen = sizeof(sockaddr_in);
        int answer_length;
        answer_length = recvfrom(client_socket_, buffer, BUFFER_LENGTH, 0, (sockaddr*)&addr_server_, &slen);

        if (answer_length == SOCKET_ERROR) {
            spdlog::warn("client recv from server failed, error code: {}", WSAGetLastError());
            continue;
        } else {
            spdlog::debug("client receive message from {}:{}", inet_ntoa(addr_server_.sin_addr), htons(addr_server_.sin_port));
        }

        if( ProcessServerMessage(std::string(buffer), client_transfer_message) < 0) {
            spdlog::error("client process server message fail");
            return -1;
        } else {
            spdlog::debug("process server message success");
        }

        std::thread send_thread([client_transfer_message, this](){
            char temp_buffer[BUFFER_LENGTH];
            ZeroMemory(temp_buffer, BUFFER_LENGTH);
            strcpy(temp_buffer, client_transfer_message.c_str());
            SendClientMessage(temp_buffer);
        });
        send_thread.detach();
    }

    closesocket(client_socket_);
    spdlog::info("close client socket");
    return 0;
}


int GBNClient::ProcessServerMessage(const std::string& buffer, std::string& client_message) {
    TransferMessage server_message;
    if (StringToMessage(buffer, server_message) == -1) {
        spdlog::error("parse server message string failed");
        return -1;
    } else {
        spdlog::info("server->client: seq: {} ack: {} data: {}", server_message.seq, server_message.ack, server_message.data);
    }

    if (server_message.seq != base_) {
        server_message.ack = base_;
        server_message.seq = 0;
        server_message.data = "hello from client";
    }  else {
        server_message.ack = server_message.seq;
        server_message.seq = 0;
        server_message.data = "hello from client";
        base_++;
    }


    client_message = MessageToString(server_message);
    return 0;
}

int GBNClient::SendClientMessage(char* buffer) {
    Sleep(5000);
    // send message
    if (sendto(client_socket_, buffer, strlen(buffer), 0, (sockaddr*)&addr_server_, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        spdlog::error("client send message fail, error code: {}", WSAGetLastError());
        return -1;
    }
    return 0;
}