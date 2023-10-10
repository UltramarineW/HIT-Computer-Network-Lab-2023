#ifndef HTTP_HEADER_MESSAGE_H
#define HTTP_HEADER_MESSAGE_H

#include <winsock2.h>
#include <iostream>


struct HttpHeader{
    // request
    char method[8];
    char url[1024];
    int port;
    char host[1024];
    char user_agent[1024];
    char cookie[1024*10];
    // response
    size_t state_word;
    size_t body_len;
    char if_modified_since[1024];
    HttpHeader () {
        ZeroMemory(this, sizeof(HttpHeader));
    }

    friend std::ostream& operator<< (std::ostream& out, const HttpHeader& header)
    {
        out << "method: " << header.method
            << "\nurl: " << header.url
            << "\nport: " << header.port
            << "\nhost: " << header.host
            << "\nuser_agent: " << header.user_agent
            << "\ncookie: " << header.cookie;
        return out;
    }
};

struct HttpMessage {
    HttpHeader* header;
    char *body;
};


#endif