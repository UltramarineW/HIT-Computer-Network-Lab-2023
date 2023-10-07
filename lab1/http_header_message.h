#ifndef HTTP_HEADER_MESSAGE_H
#define HTTP_HEADER_MESSAGE_H

#include <winsock2.h>

struct HTTPHeader{
    char method[4];
    char url[1024];
    char host[1024];
    char cookie[1024*10];
    HTTPHeader () {
        ZeroMemory(this, sizeof(HTTPHeader));
    }
};

#endif