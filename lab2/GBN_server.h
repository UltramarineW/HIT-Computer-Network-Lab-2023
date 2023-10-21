//
// Created by wujiayang on 2023/10/17.
//

#ifndef COMPUTER_NETWORK_LAB_GBN_SERVER_H
#define COMPUTER_NETWORK_LAB_GBN_SERVER_H

#include <string>
#include <spdlog/spdlog.h>
#include <set>
#include <algorithm>
#include <winsock2.h>
#include <queue>
#include "utils.h"

#define SEND_WIND_SIZE 4

class GBNServer {
public:
    GBNServer(const unsigned int &port, std::string ip);

    int Start();

private:
    int ProcessClientMessage(const std::string &message, int &ack);

    int InitServerSocket();

    int SetSocketTimeout();

    int SendServerMessage(char *temp_message);

    int HandshakeProcess();

    unsigned int port_;
    std::string ip_;
    SOCKET server_socket_;
    sockaddr_in addr_server_;
    sockaddr_in addr_client_;

    std::unique_ptr<std::vector<std::string>> send_data_;
    std::mutex mtx_;
    int send_base_;
    int next_seq_num_;
    std::ofstream receive_file_;
    std::unique_ptr<std::vector<std::string>> receive_data_;
    std::set<int> ack_windows_;
};


#endif //COMPUTER_NETWORK_LAB_GBN_SERVER_H
