//
// Created by wujiayang on 2023/10/18.
//

#include "utils.h"

std::string MessageToString(const TransferMessage &message) {
    return fmt::format("Seq: {}\r\nAck: {}\r\nData: {}", message.seq, message.ack, message.data);
}

int StringToMessage(const std::string &transfer_string, TransferMessage &message) {
    std::regex pattern(R"(Seq: (\d+)\r\nAck: (\d+)\r\nData: (.*)$)");
    std::smatch matches;

    if (std::regex_search(transfer_string, matches, pattern)) {
        if (matches.size() == 4) {
            message.seq = std::stoi(matches[1]);
            message.ack = std::stoi(matches[2]);
            message.data = matches[3];
            return 0;
        }
        return -1;
    } else {
        return -1;
    }
}

int GetRandomInteger(int min_value, int max_value) {
    std::random_device rd;
    std::mt19937 mt(rd());

    std::uniform_int_distribution<int> dist(min_value, max_value);
    return dist(mt);
}


