//
// Created by wujiayang on 2023/10/17.
//

#include "GBN_client.h"

#include <utility>

GBNClient::GBNClient(const unsigned int &port, std::string ip) : port_(port),
                                                                 ip_(std::move(ip)),
                                                                 receive_base_(0),
                                                                 next_seq_num_(0),
                                                                 send_base_(0),
                                                                 count_(0),
                                                                 receive_file_(
                                                                         R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab2\client_receive_text.txt)") {
    spdlog::debug("udp client start");

    send_data_ = std::make_unique<std::vector<std::string>>();
//    for (int i = 0; i < 20; i++) {
//        std::string data_package = "client_data_package_" + std::to_string(i);
//        send_data_->push_back(std::move(data_package));
//    }
    if (!receive_file_.is_open()) {
        spdlog::warn("client receive file doesn't open");
    } else {
        spdlog::debug("client receive file open successfully");
    }

    std::ifstream file(R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab2\client_send_text.txt)");
    if (!file.is_open()) {
        spdlog::error("can't open file: {}", "client_send_text.txt");
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
    spdlog::info("client send text read success, vector size: {}", send_data_->size());
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
    int client_recv_timeout = 3 * 1000; // 3s
    if (setsockopt(client_socket_, SOL_SOCKET, SO_RCVTIMEO, (char *) &client_recv_timeout,
                   sizeof(client_recv_timeout)) == -1) {
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
    if (sendto(client_socket_, buffer, strlen(buffer), 0, (sockaddr *) &addr_server_, sizeof(sockaddr_in)) ==
        SOCKET_ERROR) {
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

    TransferMessage client_message;
    char buffer[BUFFER_LENGTH];

    // start communication
    while (true) {
        // recv answer
        ZeroMemory(buffer, BUFFER_LENGTH);
        int slen = sizeof(sockaddr_in);
        int answer_length;
        answer_length = recvfrom(client_socket_, buffer, BUFFER_LENGTH, 0,
                                 (sockaddr *) &addr_server_, &slen);

        if (answer_length == SOCKET_ERROR) {
            auto error_code = WSAGetLastError();
            spdlog::warn("client recv from server timeout, error code: {}", error_code);
            next_seq_num_ = send_base_;
            if (error_code == 10093) {
                break;
            }
            continue;
        } else {
            spdlog::debug("client receive message from {}:{}",
                          inet_ntoa(addr_server_.sin_addr),
                          htons(addr_server_.sin_port));
        }

        if (ProcessServerMessage(std::string(buffer)) < 0) {
            spdlog::error("client process server message fail");
            return -1;
        } else {
            spdlog::debug("process server message success");
        }
        client_message.ack = receive_base_ - 1;
        client_message.seq = std::min(send_data_->size() - 1, static_cast<unsigned long long>(receive_base_ - 1));
        client_message.data = send_data_->at(client_message.seq);

        auto client_message_string = MessageToString(client_message);

        if (count_ == 1) {
            // for package loss use
            count_ = count_ + 1;
        } else {
            std::thread send_thread([client_message_string, this]() {
                char temp_buffer[BUFFER_LENGTH];
                ZeroMemory(temp_buffer, BUFFER_LENGTH);
                strcpy(temp_buffer, client_message_string.c_str());
                SendClientMessage(temp_buffer);
            });
            send_thread.detach();
        }
    }

    closesocket(client_socket_);
    receive_file_.close();
    spdlog::info("close client socket");
    return 0;
}


int GBNClient::ProcessServerMessage(const std::string &buffer) {
    TransferMessage server_message;
    if (StringToMessage(buffer, server_message) == -1) {
        spdlog::error("parse server message string failed, message: {}", buffer);
        return -1;
    } else {
        spdlog::info("server->client: seq: {} ack: {} data: {}", server_message.seq, server_message.ack,
                     server_message.data);
    }

    if (server_message.seq == receive_base_) {
        // set loss package
        if (server_message.seq == 2 && count_ == 0) {
//        if (false) {
            count_++;
        } else {
            receive_base_++;
            if (receive_file_.is_open()) {
                receive_file_ << server_message.data;
            }
        }
    }


    return 0;
}

int GBNClient::SendClientMessage(char *buffer) {
    Sleep(1000);
    // send message
    if (sendto(client_socket_, buffer, strlen(buffer), 0, (sockaddr *) &addr_server_, sizeof(sockaddr_in)) ==
        SOCKET_ERROR) {
        spdlog::error("client send message fail, error code: {}", WSAGetLastError());
        return -1;
    }
    return 0;
}