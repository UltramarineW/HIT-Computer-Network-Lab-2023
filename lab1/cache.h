//
// Created by wujiayang on 2023/10/8.
//

#ifndef COMPUTER_NETWORK_LAB_CACHE_H
#define COMPUTER_NETWORK_LAB_CACHE_H

#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <mutex>

#include "http_message.h"

using std::string;

struct CacheInstance{
    size_t id;
    string hostname;
    string url;
    size_t body_len;
    std::string modified_gmt;
    std::hash<string> hash_str;

    CacheInstance(HttpHeader header) {
        this->id = hash_str(string(header.host) + string(header.url));
        this->hostname = header.host;
        this->url = header.url;
        this->body_len = header.body_len;
    }
};


// use singleton mode to ensure there is only one instance of Cache
class Cache {
public:
    static Cache* GetInstance();

    int Add(HttpMessage http_message, char* buffer);


    CacheInstance *Find(const string &hostname, const string &url);

private:
    Cache() = default;

    static Cache* instance_;
    static std::mutex mtx_;
    std::vector<CacheInstance> cache_map_;
};


#endif //COMPUTER_NETWORK_LAB_CACHE_H
