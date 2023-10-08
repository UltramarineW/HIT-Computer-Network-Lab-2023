//
// Created by wujiayang on 2023/10/7.
//

#ifndef COMPUTER_NETWORK_LAB_HTTP_FILTER_H
#define COMPUTER_NETWORK_LAB_HTTP_FILTER_H

#include "gflags/gflags.h"
#include "http_header_message.h"
#include <iostream>
#include <fstream>

DECLARE_int32(website_filter_protocol);
DECLARE_int32(user_filter_protocol);

class HTTPFilter {
public:
    HTTPFilter(const std::string& website_white_list_filename, const std::string& website_black_list_filename, const std::string& user_white_list_filename, const std::string& user_black_list_filename);
    bool Filter(const HTTPHeader& http_header) const;
private:
    int _website_filter_protocol;
    int _user_filter_protocol;
    std::vector<std::string> _website_white_lists;
    std::vector<std::string> _website_black_lists;
    std::vector<std::string> _user_white_lists;
    std::vector<std::string> _user_black_lists;
};


#endif //COMPUTER_NETWORK_LAB_HTTP_FILTER_H
