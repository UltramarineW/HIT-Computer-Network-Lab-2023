//
// Created by wujiayang on 2023/10/17.
//

#include "SR_client.h"

#include <utility>


SRClient::SRClient(const unsigned int &port, std::string ip) : port_(port),
                                                               ip_(std::move(ip)),
                                                               receive_base_(0),
                                                               count_{},
                                                               receive_file_(
                                                                       R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab2\client_receive_text.txt)") {
    spdlog::debug("udp client start");

    send_data_ = std::make_unique<std::vector<std::string>>();
    // for test
//    for (int i = 0; i < 20; i++) {
//        std::string data_package = "client_data_package_" + std::to_string(i);
//        send_data_->push_back(std::move(data_package));
//    }

    // file transfer
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


int SRClient::InitClientServerSocket() {
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

int SRClient::HandshakeProcess() {
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

int SRClient::Start() {

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
            if (error_code == 10093) {
                break;
            }
            continue;
        } else {
            spdlog::debug("client receive message from {}:{}",
                          inet_ntoa(addr_server_.sin_addr),
                          htons(addr_server_.sin_port));
        }

        int last_receive_base = receive_base_;
        int over_ack = 0;

        int err = ProcessServerMessage(std::string(buffer), over_ack);
        if (err < 0) {
            spdlog::error("client process server message fail");
            return -1;
        } else if (err == 1) {
            continue;
        } else {
            spdlog::debug("process server message success");
        }
        // set ack random loss
        if (GetRandomInteger(1, 100) <= ACK_LOSS_RATE) {
            spdlog::critical("ack loss");
            continue;
        }

        // ack message in SR receive windows
        if (over_ack > receive_base_ || over_ack < receive_base_) {
            client_message.ack = over_ack;
            client_message.seq = std::min(send_data_->size() - 1, static_cast<unsigned long long>(over_ack));
            client_message.data = send_data_->at(client_message.seq);
            auto client_message_string = MessageToString(client_message);

            std::thread send_thread([client_message_string, this]() {
                char temp_buffer[BUFFER_LENGTH];
                ZeroMemory(temp_buffer, BUFFER_LENGTH);
                strcpy(temp_buffer, client_message_string.c_str());
                SendClientMessage(temp_buffer);
            });
            send_thread.detach();
        }
        // ack message for normal SR receive windows
        for (int i = last_receive_base; i < receive_base_; i++) {
            auto it = receive_buffer_data_.find(i);
            if (it != receive_buffer_data_.end()) {
                receive_buffer_data_.erase(it);
                continue;
            }
            client_message.ack = i;
            client_message.seq = std::min(send_data_->size() - 1, static_cast<unsigned long long>(i));
            client_message.data = send_data_->at(client_message.seq);

            auto client_message_string = MessageToString(client_message);

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


int SRClient::ProcessServerMessage(const std::string &buffer, int &ack) {
    TransferMessage server_message;
    if (StringToMessage(buffer, server_message) == -1) {
        spdlog::error("parse server message string failed, message: {}", buffer);
        return -1;
    } else {
        spdlog::info("server->client: seq: {} ack: {} data: {}", server_message.seq, server_message.ack,
                     server_message.data);
    }

    // random package loss return
    if (GetRandomInteger(1, 100) <= PACKAGE_LOSS_RATE) {
        spdlog::critical("package {} loss", server_message.seq);
        return 1;
    }

    if (server_message.seq == receive_base_) {
        receive_base_++;
        if (receive_file_.is_open()) {
            receive_file_ << server_message.data;
        }
        while (true) {
            auto it = receive_buffer_data_.find(receive_base_);
            if (it == receive_buffer_data_.end()) break;

            receive_base_++;
            if (receive_file_.is_open()) {
                receive_file_ << it->second;
            }
        }
        ack = receive_base_;
    } else if (server_message.seq < receive_base_ + RECV_WIND_SIZE && server_message.seq > receive_base_) {
        receive_buffer_data_[server_message.seq] = server_message.data;
        if (receive_buffer_data_.size() >= RECV_WIND_SIZE) {
            spdlog::error("SR error: receive buffer data size > receive window size");
            return -1;
        }
        ack = server_message.seq;
    } else if (server_message.seq >= receive_base_ + RECV_WIND_SIZE) {
        ack = receive_base_;
    } else {
        // ack loss situation
        ack = server_message.seq;
    }
    return 0;
}

int SRClient::SendClientMessage(char *buffer) {
    Sleep(1000);
    // send message
    if (sendto(client_socket_, buffer, strlen(buffer), 0, (sockaddr *) &addr_server_, sizeof(sockaddr_in)) ==
        SOCKET_ERROR) {
        spdlog::error("client send message fail, error code: {}", WSAGetLastError());
        return -1;
    }
    return 0;
}