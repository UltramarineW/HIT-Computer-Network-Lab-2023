//
// Created by wujiayang on 2023/10/17.
//

#include "GBN_server.h"
#include "transfer_massage.h"

GBNServer::GBNServer(const unsigned int& port, std::string ip) : port_(port), ip_(std::move(ip)){
    spdlog::info("udp server start");
}

int GBNServer::Start() const {
    SOCKET server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == INVALID_SOCKET) {
        spdlog::error("create server socket error");
        return -1;
    } else {
        spdlog::info("server socket create success: {}", server_socket);
    }
    // set server recv timeout
    int server_recv_timeout = 2 * 1000; // 2s
    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&server_recv_timeout, sizeof(server_recv_timeout)) == -1) {
        spdlog::error("set receive timeout fail");
    }

    sockaddr_in addr_server{}, addr_client{};
    addr_server.sin_addr.S_un.S_addr = INADDR_ANY;
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(port_);

    if (bind(server_socket, (sockaddr*)&addr_server, sizeof(addr_server)) == SOCKET_ERROR) {
        spdlog::error("bind socket error, error code: {}", WSAGetLastError());
        return -1;
    } else {
        spdlog::info("server bind socket done");
    }

    // start communication
    while (true) {
        // receive message from client
        char message[BUFFER_LENGTH];
        ZeroMemory(message, BUFFER_LENGTH);
        int message_len;
        int slen = sizeof(sockaddr_in);



        // server receive message
        message_len = recvfrom(server_socket, message, BUFFER_LENGTH, 0, (sockaddr*)&addr_client, &slen);
        if (message_len == SOCKET_ERROR) {
            spdlog::warn("server recv message from client failed, error code: {}", WSAGetLastError());
            continue;
        } else {
            spdlog::debug("server receive message from {}:{}",
                                inet_ntoa(addr_client.sin_addr),
                                ntohs(addr_client.sin_port));
        }
        // Parse transfer message
        TransferMassage client_message;
        if (StringToMessage(std::string(message), client_message) == -1) {
            spdlog::error("parse server message string failed");
            return -1;
        } else {
            spdlog::info("client->server: seq: {} ack: {} data: {}", client_message.seq, client_message.ack, client_message.data);
        }

        if (ProcessClientMessage(client_message) < 0) {
            spdlog::error("processing client message failed");
        } else {
            spdlog::debug("process client message success");
        }

        auto server_message = MessageToString(client_message);

        Sleep(1000);
       // server send message
        strcpy(message, server_message.c_str());
        if (sendto(server_socket, message, strlen(message), 0, (sockaddr*)&addr_client, sizeof(sockaddr_in)) == SOCKET_ERROR) {
            spdlog::error("server sendto() error, error code: {}", WSAGetLastError());
        } else {
            spdlog::debug("server send message success");
        }


    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

int GBNServer::ProcessClientMessage(TransferMassage &message) const {
    message.data = "hello from server";
    message.ack = message.seq;
    message.seq = (message.seq + 1) % (SEQ_SIZE);
    return 0;
}

