//
// Created by wujiayang on 2023/10/18.
//

#ifndef COMPUTER_NETWORK_LAB_UTILS_H
#define COMPUTER_NETWORK_LAB_UTILS_H

#include <string>
#include <fmt/format.h>
#include <fmt/core.h>
#include "transfer_message.h"
#include <regex>
#include <spdlog/spdlog.h>
#include <random>

std::string MessageToString(const TransferMessage& message);

int  StringToMessage(const std::string& transfer_string, TransferMessage& message);

int GetRandomInteger(int min_value, int max_value);

int TestFunction();

#endif //COMPUTER_NETWORK_LAB_UTILS_H
