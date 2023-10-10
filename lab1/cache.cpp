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

int Cache::Add(HttpMessage http_message) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = cache_map_.begin();
    while (it != cache_map_.end()) {
        if (it->hostname == http_message.header->host && it->url == http_message.header->url) {
            break;
        }
        it++;
    }

    size_t instance_id;

    if (it == cache_map_.end()) {
        CacheInstance instance{*http_message.header};
        cache_map_.push_back(instance);
        instance_id = instance.id;
    } else {
        it->body_len = http_message.header->body_len;
        instance_id = it->id;
    }
    char filename[100]; // 保存文件名
	snprintf(filename, sizeof(filename), "E:/HIT_Project/HIT-Computer-Network-Lab-2023/lab1/cache/%zu", instance_id); // 保存在cache目录下
    std::cout << filename << std::endl;

    std::ofstream cache_file((std::string(filename)));
    if (!cache_file.is_open()) {
        std::cerr << "[Error] Cache file created fail" << std::endl;
        return -1;
    }
    cache_file << std::string(http_message.body);
    cache_file.close();


//	FILE *f = fopen(filename, "w");
//	fwrite(http_message.body, http_message.header->body_len, 1, f);
//	fclose(f);
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