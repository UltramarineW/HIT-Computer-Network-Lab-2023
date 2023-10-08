//
// Created by wujiayang on 2023/10/7.
//

#include "http_filter.h"

HTTPFilter::HTTPFilter(const std::string& website_white_list_filename, const std::string& website_black_list_filename, const std::string& user_white_list_filename, const std::string& user_black_list_filename) {

    _website_filter_protocol = FLAGS_website_filter_protocol;
    _user_filter_protocol = FLAGS_user_filter_protocol;
    std::ifstream website_white_list_file(website_white_list_filename);
    std::ifstream website_black_list_file(website_black_list_filename);
    std::ifstream user_white_list_file(user_white_list_filename);
    std::ifstream user_black_list_file(user_black_list_filename);

    if (website_white_list_file.is_open()) {
        std::string line;
        while (std::getline(website_white_list_file, line)) {
            _website_white_lists.push_back(line);
        }
        website_white_list_file.close();
    } else {
        std::cerr << "[Error] Open white list file failed " << std::endl;
    }
    if (website_black_list_file.is_open()) {
        std::string line;
        while (std::getline(website_black_list_file, line)) {
            _website_black_lists.push_back(line);
        }
        website_black_list_file.close();
    } else {
        std::cerr << "[Error] Open black list file failed" << std::endl;
    }

    if (user_white_list_file.is_open()) {
        std::string line;
        while (std::getline(user_white_list_file, line)) {
            _user_white_lists.push_back(line);
        }
        user_white_list_file.close();
    } else {
        std::cerr << "[Error] Open white list file failed " << std::endl;
    }
    if (user_black_list_file.is_open()) {
        std::string line;
        while (std::getline(user_black_list_file, line)) {
            _user_black_lists.push_back(line);
        }
        user_black_list_file.close();
    } else {
        std::cerr << "[Error] Open black list file failed" << std::endl;
    }
}

bool HTTPFilter::Filter(const HTTPHeader& http_header) const {
    bool pass = true;
    switch (_website_filter_protocol) {
        // no protocol
        case 0:
            break;
        // white list protocol
        case 1:
            pass = false;
            for (auto& line : _website_white_lists) {
                std::string host = http_header.host;
                if (host.find(line)) {
                    pass = true;
                }
            }
            break;
        // black list protocol
        case 2:
            for (auto& line : _website_black_lists) {
                std::string host = http_header.host;
                size_t found = host.find(line);
                if (found != std::string::npos) {
                    std::cout << host << "    " << line << std::endl;
                    pass = false;
                }
            }
        default:
            break;
    }
    if (!pass) return false;
    switch (_user_filter_protocol) {
        // no protocol
        case 0:
            break;
            // white list protocol
        case 1:
            pass = false;
            for (auto& line : _user_white_lists) {
                std::string user = http_header.user_agent;
                if (user.find(line)) {
                    pass = true;
                }
            }
            break;
            // black list protocol
        case 2:
            for (auto& line : _user_black_lists) {
                std::string user = http_header.user_agent;
                if (user.find(line)) {
                    pass = false;
                }
            }
        default:
            break;
    }
    return pass;
}