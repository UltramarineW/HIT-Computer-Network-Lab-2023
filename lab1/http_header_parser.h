#ifndef HTTP_HEAD_PARSER_H
#define HTTP_HEAD_PARSER_H

#include "http_message.h"
#include <memory>
#include <iostream>

#define HTTP_PORT 80
#define HTTPS_PORT 443

class HttpHeaderParser {

public:
    // parser constructor
    HttpHeaderParser();

    // parse http request message
    bool ParseRequest(char *buffer);

    // parse http response message
    bool ParseResponse(char *buffer);

    // return http header message after parse
    HttpHeader GetHeaderMessage();

    // return http message
    HttpMessage GetHttpMessage();


private:
    std::unique_ptr<HttpHeader> http_header_;
    std::unique_ptr<HttpMessage> http_message_;
};

#endif