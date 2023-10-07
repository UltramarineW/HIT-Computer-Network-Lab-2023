#ifndef HTTP_HEAD_PARSER_H
#define HTTP_HEAD_PARSER_H

#include "http_header_message.h"
#include <memory>
#include <iostream>

class HTTP_Header_Parser{

public:
    // parser constructor
    HTTP_Header_Parser();

    // parse http message
    bool Parse(char *buffer);

    // return http message after parse
    HTTPHeader GetHeaderMessage();

private:
    std::unique_ptr<HTTPHeader> http_header;
};

#endif