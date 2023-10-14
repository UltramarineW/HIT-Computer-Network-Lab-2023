//
// Created by wujiayang on 2023/10/8.
//

#include "cache.h"

Cache* Cache::instance_ = nullptr;
std::mutex Cache::mtx_;

Cache* Cache::GetInstance() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (instance_ == nullptr) {
        instance_ = new Cache();
    }
    return instance_;
}

int Cache::Add(HttpMessage http_message, char* buffer) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = cache_map_.begin();
    while (it != cache_map_.end()) {
        if (it->hostname == http_message.header->host && it->url == http_message.header->url) {
            break;
        }
        it++;
    }

    auto buffer_string = std::string(buffer);
    std::string last_modified_string;
    auto last_modified = buffer_string.find("Last-Modified: ");
    if (last_modified == std::string::npos) {
        std::cout << "[Info] Can't find last modified segment" << std::endl;
        return 0;
    } else {
        auto buffer_substring = buffer_string.substr(last_modified + 15);
        auto last_modified_end = buffer_substring.find("\r\n");
        last_modified_string = buffer_substring.substr(0, last_modified_end);
    }

    size_t instance_id;

    if (it == cache_map_.end()) {
        CacheInstance instance{*http_message.header};
        instance_id = instance.id;
        instance.modified_gmt = last_modified_string;
        cache_map_.push_back(instance);
    } else {
        it->body_len = http_message.header->body_len;
        instance_id = it->id;
        it->modified_gmt = last_modified_string;
    }
    char filename[100]; // 保存文件名
	snprintf(filename, sizeof(filename), "E:/HIT_Project/HIT-Computer-Network-Lab-2023/lab1/cache/%zu", instance_id); // 保存在cache目录下
    std::cout << filename << std::endl;

    std::ofstream cache_file(std::string(filename), std::ios::out);
    if (!cache_file.is_open()) {
        std::cerr << "[Error] Cache file created fail" << std::endl;
        return -1;
    }
    cache_file <<  std::string(buffer);
    cache_file.close();

    return 0;
}


CacheInstance* Cache::Find(const std::string &hostname, const std::string &url) {
    auto it = cache_map_.begin();
    while (it != cache_map_.end()) {
        if (it->hostname == hostname && it->url == url) {
            return it.base();
        }
        it++;
    }
    return nullptr;
}