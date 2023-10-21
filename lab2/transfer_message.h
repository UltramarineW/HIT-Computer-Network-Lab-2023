//
// Created by wujiayang on 2023/10/18.
//

#ifndef COMPUTER_NETWORK_LAB_TRANSFER_MESSAGE_H
#define COMPUTER_NETWORK_LAB_TRANSFER_MESSAGE_H

#define BUFFER_LENGTH 1026
// the seq number is an integer between [0, 19]

struct TransferMessage {
    int seq;
    int ack;
    std::string data;
};

#endif //COMPUTER_NETWORK_LAB_TRANSFER_MESSAGE_H
