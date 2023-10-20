//
// Created by wujiayang on 2023/10/17.
//

#ifndef COMPUTER_NETWORK_LAB_GBN_SERVER_H
#define COMPUTER_NETWORK_LAB_GBN_SERVER_H

#include <string>
#include <spdlog/spdlog.h>
#include <winsock2.h>
#include "utils.h"

class GBNServer {
public:
    GBNServer(const unsigned int& port, std::string ip);
    int Start() const;

private:
    int ProcessClientMessage(TransferMassage& message) const;
//    void HandshakeProcess();
    unsigned int port_;
    std::string ip_;
    sockaddr_in addr_server_;
    sockaddr_in addr_client_;
};


#endif //COMPUTER_NETWORK_LAB_GBN_SERVER_H
