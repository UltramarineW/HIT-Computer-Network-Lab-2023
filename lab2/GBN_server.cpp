//
// Created by wujiayang on 2023/10/17.
//

#include "GBN_server.h"
#include "transfer_message.h"

GBNServer::GBNServer(const unsigned int& port, std::string ip) : port_(port), ip_(std::move(ip)), base_(0), next_seq_num_(0){
    spdlog::debug("udp server start");
    send_data_ = std::make_unique<std::vector<std::string>>();
    for (int i = 0; i < 100; i++) {
        std::string data_package = "data_package_" + std::to_string(i);
        send_data_->push_back(std::move(data_package));
    }
}

int GBNServer::Start() {
    // init server socket
    if (InitServerSocket() < 0) {
        return -1;
    }
    // handshake with client
    if (HandshakeProcess() < 0) {
        spdlog::error("handshake with client error");
        return -1;
    } else {
        spdlog::debug("handshake with client success");
    }

    char message[BUFFER_LENGTH];
    ZeroMemory(message, BUFFER_LENGTH);
    TransferMessage server_message;

    // start communication
    while (true) {
        if (base_ == send_data_->size()) break;
        // server send message
        while (next_seq_num_ - base_ + 1 <= SEND_WIND_SIZE && next_seq_num_ < send_data_->size()) {
            server_message.seq = next_seq_num_;
            server_message.ack = 0;
            server_message.data = send_data_->at(next_seq_num_);
            auto server_message_string = MessageToString(server_message);

            std::thread send_thread([server_message_string, this](){
                char temp_message[BUFFER_LENGTH];
                strcpy(temp_message, server_message_string.c_str());
                SendServerMessage(temp_message);
            });
            Sleep(100);
            send_thread.detach();
            next_seq_num_++;
        }

        // receive message from client
        int message_len;
        int slen = sizeof(sockaddr_in);
        ZeroMemory(message, BUFFER_LENGTH);

        message_len = recvfrom(server_socket_, message, BUFFER_LENGTH, 0, (sockaddr*)&addr_client_, &slen);
        if (message_len == SOCKET_ERROR) {
            spdlog::warn("server recv message from client timeout, error code: {}", WSAGetLastError());
            next_seq_num_ = base_;
            continue;
        } else {
            spdlog::debug("server receive message from {}:{}",
                                inet_ntoa(addr_client_.sin_addr),
                                ntohs(addr_client_.sin_port));
        }

        if (ProcessClientMessage(std::string(message)) < 0) {
            spdlog::error("processing client message failed");
            return -1;
        } else {
            spdlog::debug("process client message success");
        }
    }
    closesocket(server_socket_);
    WSACleanup();
    return 0;
}

int GBNServer::ProcessClientMessage(const std::string& message) {
    // Parse transfer message
    TransferMessage client_message;
    if (StringToMessage(message, client_message) == -1) {
        spdlog::error("parse client message string failed, message: {}", message);
        return -1;
    } else {
        spdlog::info("client->server: seq: {} ack: {} data: {}", client_message.seq, client_message.ack, client_message.data);
    }

    base_ = client_message.ack + 1;
    return 0;
}


int GBNServer::InitServerSocket() {
    server_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket_ == INVALID_SOCKET) {
        spdlog::error("create server socket error");
        return -1;
    } else {
        spdlog::info("server socket create success: {}", server_socket_);
    }

    addr_server_.sin_addr.S_un.S_addr = INADDR_ANY;
    addr_server_.sin_family = AF_INET;
    addr_server_.sin_port = htons(port_);

    if (bind(server_socket_, (sockaddr*)&addr_server_, sizeof(addr_server_)) == SOCKET_ERROR) {
        spdlog::error("bind socket error, error code: {}", WSAGetLastError());
        return -1;
    } else {
        spdlog::debug("server bind socket done");
    }
    return 0;
}

int GBNServer::HandshakeProcess() {
    // receive message from client
    char message[BUFFER_LENGTH];
    ZeroMemory(message, BUFFER_LENGTH);
    int message_len;
    int slen = sizeof(sockaddr_in);

    // server receive message
    message_len = recvfrom(server_socket_, message, BUFFER_LENGTH, 0, (sockaddr*)&addr_client_, &slen);
    if (message_len == SOCKET_ERROR) {
        spdlog::warn("server recv message from client failed, error code: {}", WSAGetLastError());
    } else {
        spdlog::debug("server receive message from {}:{}",
                      inet_ntoa(addr_client_.sin_addr),
                      ntohs(addr_client_.sin_port));
    }

    if (SetSocketTimeout() < 0) {
        return -1;
    }
    return 0;
}

int GBNServer::SetSocketTimeout() {
    // set server recv timeout
    int server_recv_timeout = 3 * 1000; // 2s
    if (setsockopt(server_socket_, SOL_SOCKET, SO_RCVTIMEO, (char*)&server_recv_timeout, sizeof(server_recv_timeout)) == -1) {
        spdlog::error("set receive timeout fail");
        return -1;
    } else {
        spdlog::debug("set server socket timeout: {}", server_recv_timeout);
    }
    return 0;
}

int GBNServer::SendServerMessage(char *temp_message) {
    Sleep(1000);
    if (sendto(server_socket_, temp_message, strlen(temp_message), 0, (sockaddr*)&addr_client_, sizeof(sockaddr_in)) == SOCKET_ERROR) {
        spdlog::error("server sendto() error, error code: {}", WSAGetLastError());
    } else {
        spdlog::debug("server send message success");
    }
    return 0;
}