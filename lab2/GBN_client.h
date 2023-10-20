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
#include <fmt/format.h>
#include "transfer_message.h"
#include "utils.h"


using std::string;

class GBNClient{
public:
    GBNClient(const unsigned int& port, std::string  ip);
    int Start();

private:
    int ProcessServerMessage (const std::string& message, std::string& client_message);
    int InitClientServerSocket();
    int HandshakeProcess();
    int SendClientMessage(char* buffer);

    unsigned int port_;
    string ip_;
    SOCKET client_socket_;
    sockaddr_in addr_server_;
    int base_;
};


#endif //COMPUTER_NETWORK_LAB_GBN_CLIENT_H
