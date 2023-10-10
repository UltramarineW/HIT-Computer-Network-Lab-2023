#include "http_header_parser.h"

HttpHeaderParser::HttpHeaderParser() : http_header_(std::make_unique<HttpHeader>()), http_message_(std::make_unique<HttpMessage>()) {
    http_message_->header = http_header_.get();
}

bool HttpHeaderParser::ParseRequest(char *buffer) {
    char *p;
    char *ptr;
    const char *delim = "\r\n";
    p = strtok_s(buffer, delim, &ptr);
    // GET 方法
    if (p[0] == 'G')
    {
        memcpy(http_header_->method, "GET", 3);
        memcpy(http_header_->url, &p[4], strlen(p) - 13);
    }
    else if (p[0] == 'P')
    {
        memcpy(http_header_->method, "POST", 4);
        memcpy(http_header_->url, &p[5], strlen(p) - 14);
    }
    else if (p[0] == 'C')
    {
        memcpy(http_header_->method, "CONNECT", 7);
        memcpy(http_header_->url, &p[8], strlen(p) - 17);
    }


    p = strtok_s(nullptr, delim, &ptr);
    while (p)
    {
        switch (p[0])
        {
        case 'H':
            memcpy(http_header_->host, &p[6], strlen(p) - 6);
            break;
        case 'U':
            memcpy(http_header_->user_agent, &p[12], strlen(p) - 12);
            break;
        case 'C':
            if (strlen(p) > 8)
            {
                char header[8];
                ZeroMemory(header, sizeof(header));
                memcpy(header, p, 6);
                if (!strcmp(header, "Cookie"))
                {
                    memcpy(http_header_->cookie, &p[8], strlen(p) - 8);
                }
            }
            break;
        case 'I':
            if (strlen(p) > 21)
            {
                char if_modified[21];
                ZeroMemory(if_modified, sizeof(if_modified));
                memcpy(if_modified, p, 19);
                if (!strcmp(if_modified, "If-Modified-Since"))
                {
                    memcpy(http_header_->if_modified_since, &p[21], strlen(p) - 21);
                }
            }


        default:
            break;
        }
        p = strtok_s(nullptr, delim, &ptr);
    }

    // Deal with HTTPS
    http_header_->port = HTTP_PORT;
    if (!strcmp(http_header_->host + strlen(http_header_->host) - 3, "443")) {
        http_header_->port = HTTPS_PORT;
    }
    return true;
}

bool HttpHeaderParser::ParseResponse(char *buffer) {
    // get state word
    char *ptr;
    char* p = strtok_s(buffer, "\r\n", &ptr);
    p = p + 9;
    char temp_state_word[4];
    strncpy(temp_state_word, p, 3);
    temp_state_word[3] = '\0';
    http_header_->state_word = atoi(temp_state_word);

    p = strtok_s(nullptr, "\r\n", &ptr);
    while(p) {
        if (std::string(p).find(':') != std::string::npos) {
            p = strtok_s(nullptr, "\r\n", &ptr);
        } else {
            break;
        }
    }
    if (p != nullptr && ptr != nullptr) {
        strcat(p, "\r\n");
        strcat(p, ptr);
        http_message_->body = p;
        http_message_->header->body_len = strlen(p);
    } else {
        http_message_->body = nullptr;
        http_message_->header->body_len = 0;
    }
}

HttpHeader HttpHeaderParser::GetHeaderMessage() {
    return *http_header_;
}

HttpMessage HttpHeaderParser::GetHttpMessage() {
    return *http_message_;
}