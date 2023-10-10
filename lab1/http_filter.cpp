//
// Created by wujiayang on 2023/10/7.
//

#include "http_filter.h"

HttpFilter::HttpFilter(const std::string& website_white_list_filename, const std::string& website_black_list_filename, const std::string& user_white_list_filename, const std::string& user_black_list_filename) {

    website_filter_protocol_ = FLAGS_website_filter_protocol;
    user_filter_protocol_ = FLAGS_user_filter_protocol;
    std::ifstream website_white_list_file(website_white_list_filename);
    std::ifstream website_black_list_file(website_black_list_filename);
    std::ifstream user_white_list_file(user_white_list_filename);
    std::ifstream user_black_list_file(user_black_list_filename);

    if (website_white_list_file.is_open()) {
        std::string line;
        while (std::getline(website_white_list_file, line)) {
            website_white_lists_.push_back(line);
        }
        website_white_list_file.close();
    } else {
        std::cerr << "[Error] Open white list file failed " << std::endl;
    }
    if (website_black_list_file.is_open()) {
        std::string line;
        while (std::getline(website_black_list_file, line)) {
            website_black_lists_.push_back(line);
        }
        website_black_list_file.close();
    } else {
        std::cerr << "[Error] Open black list file failed" << std::endl;
    }

    if (user_white_list_file.is_open()) {
        std::string line;
        while (std::getline(user_white_list_file, line)) {
            user_white_lists_.push_back(line);
        }
        user_white_list_file.close();
    } else {
        std::cerr << "[Error] Open white list file failed " << std::endl;
    }
    if (user_black_list_file.is_open()) {
        std::string line;
        while (std::getline(user_black_list_file, line)) {
            user_black_lists_.push_back(line);
        }
        user_black_list_file.close();
    } else {
        std::cerr << "[Error] Open black list file failed" << std::endl;
    }
}

bool HttpFilter::Filter(const HttpHeader& http_header) const {
    bool pass = true;
    switch (website_filter_protocol_) {
        // no protocol
        case 0:
            break;
        // white list protocol
        case 1:
            pass = false;
            for (auto& line : website_white_lists_) {
                std::string host = http_header.host;
                size_t found = host.find(line);
                if (found != std::string::npos) {
                    pass = true;
                }
            }
            break;
        // black list protocol
        case 2:
            for (auto& line : website_black_lists_) {
                std::string host = http_header.host;
                size_t found = host.find(line);
                if (found != std::string::npos) {
                    pass = false;
                }
            }
        default:
            break;
    }
    if (!pass) return false;
    switch (user_filter_protocol_) {
        // no protocol
        case 0:
            break;
            // white list protocol
        case 1:
            pass = false;
            for (auto& line : user_white_lists_) {
                std::string user = http_header.user_agent;
                size_t found = user.find(line);
                if (found != std::string::npos) {
                    pass = true;
                }
            }
            break;
            // black list protocol
        case 2:
            for (auto& line : user_black_lists_) {
                std::string user = http_header.user_agent;
                size_t found = user.find(line);
                if (found != std::string::npos) {
                    pass = false;
                }
            }
        default:
            break;
    }
    return pass;
}