//
// Created by wujiayang on 2023/10/17.
//

#include <fstream>
#include "SR_server.h"
#include "transfer_message.h"

SRServer::SRServer(const unsigned int &port, std::string ip) : port_(port),
                                                               ip_(std::move(ip)),
                                                               send_base_(0),
                                                               next_seq_num_(0),
                                                               receive_file_(
                                                                       R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab2\server_receive_text.txt)") {
    spdlog::debug("udp server start");
    send_data_ = std::make_unique<std::vector<std::string>>();
    // for test
//    for (int i = 0; i < 20; i++) {
//        std::string data_package = "server_data_package_" + std::to_string(i);
//        send_data_->push_back(std::move(data_package));
//    }

    // file transfer
    if (!receive_file_.is_open()) {
        spdlog::warn("client receive file doesn't open");
    } else {
        spdlog::debug("client receive file open successfully");
    }

    std::ifstream file(R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab2\server_send_text.txt)");
    if (!file.is_open()) {
        spdlog::error("can't open file: {}", "server_send_text.txt");
        return;
    }
    char buffer[SEND_MESSAGE_SIZE + 1];
    while (file.read(buffer, SEND_MESSAGE_SIZE)) {
        buffer[SEND_MESSAGE_SIZE] = '\0';
        send_data_->push_back(std::string(buffer));
    }
    if (file.gcount() > 0) {
        send_data_->push_back(std::string(buffer).substr(0, file.gcount()));
    }
    file.close();

    // receive data vector
    receive_data_ = std::make_unique<std::vector<std::string>>(send_data_->size(), "");
    spdlog::info("server send text read success, vector size: {}", send_data_->size());
    count_ = 0;
}

int SRServer::Start() {
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
        if (send_base_ == send_data_->size()) break;
        // server send message
        while (next_seq_num_ - send_base_ + 1 <= SEND_WIND_SIZE && next_seq_num_ < send_data_->size()) {
            if (ack_windows_.find(next_seq_num_) != ack_windows_.end()) {
                next_seq_num_++;
                continue;
            }
            server_message.seq = next_seq_num_;
            server_message.data = send_data_->at(next_seq_num_);
            auto server_message_string = MessageToString(server_message);

            std::thread send_thread([server_message_string, this]() {
                char temp_message[BUFFER_LENGTH];
                strcpy(temp_message, server_message_string.c_str());
                SendServerMessage(temp_message);
            });
            Sleep(100);
            send_thread.detach();
            next_seq_num_++;
            spdlog::info("server SR send windows change to {}", next_seq_num_ - send_base_);
        }

        // receive message from client
        int message_len;
        int slen = sizeof(sockaddr_in);
        ZeroMemory(message, BUFFER_LENGTH);

        message_len = recvfrom(server_socket_, message, BUFFER_LENGTH, 0, (sockaddr *) &addr_client_, &slen);
        if (message_len == SOCKET_ERROR) {
            spdlog::warn("server recv message from client timeout, error code: {}", WSAGetLastError());
            next_seq_num_ = send_base_;
            continue;
        } else {
            spdlog::debug("server receive message from {}:{}",
                          inet_ntoa(addr_client_.sin_addr),
                          ntohs(addr_client_.sin_port));
        }
        // process client message
        if (ProcessClientMessage(std::string(message), server_message.ack) < 0) {
            spdlog::error("processing client message failed");
            return -1;
        } else {
            spdlog::debug("process client message success");
        }
    }

    closesocket(server_socket_);
    for (const auto &it: *receive_data_) {
        receive_file_ << it;
    }
    receive_file_.close();
    spdlog::info("close server socket");
    WSACleanup();
    return 0;
}

int SRServer::ProcessClientMessage(const std::string &message, int &ack) {
    // Parse transfer message
    TransferMessage client_message;
    if (StringToMessage(message, client_message) == -1) {
        spdlog::error("parse client message string failed, message: {}", message);
        return -1;
    } else {
        spdlog::info("client->server: seq: {} ack: {} data: {}", client_message.seq, client_message.ack,
                     client_message.data);
    }
    receive_data_->at(client_message.seq) = client_message.data;
    ack_windows_.insert(client_message.ack);
    ack = client_message.seq;

    if (send_base_ == client_message.ack) {
        int temp = client_message.ack + 1;
        while (ack_windows_.find(temp) != ack_windows_.end()) {
            ack_windows_.erase(temp);
            temp += 1;
        }
        send_base_ = temp;
        spdlog::info("server SR send windows change to {}", next_seq_num_ - send_base_);
    }
    return 0;
}


int SRServer::InitServerSocket() {
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

    if (bind(server_socket_, (sockaddr *) &addr_server_, sizeof(addr_server_)) == SOCKET_ERROR) {
        spdlog::error("bind socket error, error code: {}", WSAGetLastError());
        return -1;
    } else {
        spdlog::debug("server bind socket done");
    }
    return 0;
}

int SRServer::HandshakeProcess() {
    // receive message from client
    char message[BUFFER_LENGTH];
    ZeroMemory(message, BUFFER_LENGTH);
    int message_len;
    int slen = sizeof(sockaddr_in);

    // server receive message
    message_len = recvfrom(server_socket_, message, BUFFER_LENGTH, 0, (sockaddr *) &addr_client_, &slen);
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

int SRServer::SetSocketTimeout() {
    // set server recv timeout
    int server_recv_timeout = 3 * 1000; // 2s
    if (setsockopt(server_socket_, SOL_SOCKET, SO_RCVTIMEO, (char *) &server_recv_timeout,
                   sizeof(server_recv_timeout)) == -1) {
        spdlog::error("set receive timeout fail");
        return -1;
    } else {
        spdlog::debug("set server socket timeout: {}", server_recv_timeout);
    }
    return 0;
}

int SRServer::SendServerMessage(char *temp_message) {
    Sleep(1000);
    if (sendto(server_socket_, temp_message, strlen(temp_message), 0, (sockaddr *) &addr_client_,
               sizeof(sockaddr_in)) == SOCKET_ERROR) {
        spdlog::error("server sendto() error, error code: {}", WSAGetLastError());
    } else {
        spdlog::debug("server send message success");
    }
    return 0;
}