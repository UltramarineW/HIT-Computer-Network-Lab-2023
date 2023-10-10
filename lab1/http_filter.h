//
// Created by wujiayang on 2023/10/7.
//

#ifndef COMPUTER_NETWORK_LAB_HTTP_FILTER_H
#define COMPUTER_NETWORK_LAB_HTTP_FILTER_H

#include "gflags/gflags.h"
#include "http_message.h"
#include <iostream>
#include <fstream>

DECLARE_int32(website_filter_protocol);
DECLARE_int32(user_filter_protocol);

class HttpFilter {
public:
    HttpFilter(const std::string& website_white_list_filename, const std::string& website_black_list_filename, const std::string& user_white_list_filename, const std::string& user_black_list_filename);
    [[nodiscard]] bool Filter(const HttpHeader& http_header) const;
private:
    int website_filter_protocol_;
    int user_filter_protocol_;
    std::vector<std::string> website_white_lists_;
    std::vector<std::string> website_black_lists_;
    std::vector<std::string> user_white_lists_;
    std::vector<std::string> user_black_lists_;
};


#endif //COMPUTER_NETWORK_LAB_HTTP_FILTER_H
