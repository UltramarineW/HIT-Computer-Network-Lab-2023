//
// Created by wujiayang on 2023/10/18.
//

#ifndef COMPUTER_NETWORK_LAB_TRANSFER_MASSAGE_H
#define COMPUTER_NETWORK_LAB_TRANSFER_MASSAGE_H

#define BUFFER_LENGTH 1026
// the seq number is an integer between [0, 19]
#define SEQ_MIN 0
#define SEQ_MAX 19

struct TransferMassage{
    int seq;
    int ack;
    std::string data;
};

#endif //COMPUTER_NETWORK_LAB_TRANSFER_MASSAGE_H
