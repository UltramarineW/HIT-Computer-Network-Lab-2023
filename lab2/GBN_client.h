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
#include "transfer_massage.h"
#include "utils.h"


using std::string;

class GBNClient{
public:
    GBNClient(const unsigned int& port, std::string  ip);
    int Start();

private:
    int ProcessServerMessage (TransferMassage& message) const;
    unsigned int port_;
    string ip_;
};


#endif //COMPUTER_NETWORK_LAB_GBN_CLIENT_H
