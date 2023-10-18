//
// Created by wujiayang on 2023/10/17.
//

#ifndef COMPUTER_NETWORK_LAB_STOP_WAIT_SERVER_H
#define COMPUTER_NETWORK_LAB_STOP_WAIT_SERVER_H

#include <string>
#include <spdlog/spdlog.h>
#include <winsock2.h>
#include "utils.h"

class StopWaitServer {
public:
    StopWaitServer(const unsigned int& port, std::string ip);
    int Start() const;

private:
    int ProcessClientMessage(TransferMassage& message) const;
    unsigned int port_;
    std::string ip_;

};


#endif //COMPUTER_NETWORK_LAB_STOP_WAIT_SERVER_H
