#ifndef HTTP_HEADER_MESSAGE_H
#define HTTP_HEADER_MESSAGE_H

#include <winsock2.h>
#include <iostream>


struct HTTPHeader{
    char method[8];
    char url[1024];
    int port;
    char host[1024];
    char cookie[1024*10];
    HTTPHeader () {
        ZeroMemory(this, sizeof(HTTPHeader));
    }
    friend std::ostream& operator<< (std::ostream& out, const HTTPHeader& header)
    {
        out << "method: " << header.method
            << "\nurl: " << header.url
            << "\nport: " << header.port
            << "\nhost: " << header.host
            << "\ncookie: " << header.cookie;
        return out;
    }
};

#endif