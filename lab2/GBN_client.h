//
// Created by wujiayang on 2023/10/17.
//

#ifndef COMPUTER_NETWORK_LAB_GBN_CLIENT_H
#define COMPUTER_NETWORK_LAB_GBN_CLIENT_H

#include <string>
#include <random>
#include <spdlog/spdlog.h>
#include <winsock2.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <fmt/format.h>
#include <fstream>
#include "transfer_message.h"
#include "utils.h"

#define RECV_WIND_SIZE 4

using std::string;

class GBNClient {
public:
    GBNClient(const unsigned int &port, std::string ip);

    int Start();

private:
    int ProcessServerMessage(const std::string &message, int &ack);

    int InitClientServerSocket();

    int HandshakeProcess();

    int SendClientMessage(char *buffer);

    unsigned int port_;
    string ip_;
    SOCKET client_socket_;
    sockaddr_in addr_server_;

    std::unique_ptr<std::vector<std::string>> send_data_;
    std::map<int, std::string> receive_buffer_data_;
    std::ofstream receive_file_;
    int receive_base_;
    // use for package loss
    int count_;
};


#endif //COMPUTER_NETWORK_LAB_GBN_CLIENT_H
