#include "http_header_parser.h"

HTTP_Header_Parser::HTTP_Header_Parser() {
    http_header = std::make_unique<HTTPHeader>();
}

bool HTTP_Header_Parser::Parse(char *buffer) {
    char *p;
    char *ptr;
    const char *delim = "\r\n";
    p = strtok_s(buffer, delim, &ptr);
    // GET 方法
    if (p[0] == 'G')
    {
        memcpy(http_header->method, "GET", 3);
        memcpy(http_header->url, &p[4], strlen(p) - 13);
    }
    else if (p[0] == 'P')
    {
        memcpy(http_header->method, "POST", 4);
        memcpy(http_header->url, &p[5], strlen(p) - 14);
    }
    else if (p[0] == 'C')
    {
        memcpy(http_header->method, "CONNECT", 7);
        memcpy(http_header->url, &p[8], strlen(p) - 17);
    }


    p = strtok_s(nullptr, delim, &ptr);
    while (p)
    {
        switch (p[0])
        {
        case 'H':
            memcpy(http_header->host, &p[6], strlen(p) - 6);
            break;
        case 'U':
            memcpy(http_header->user_agent, &p[12], strlen(p) - 12);
            break;
        case 'C':
            if (strlen(p) > 8)
            {
                char header[8];
                ZeroMemory(header, sizeof(header));
                memcpy(header, p, 6);
                if (!strcmp(header, "Cookie"))
                {
                    memcpy(http_header->cookie, &p[8], strlen(p) - 8);
                }
            }
            break;

        default:
            break;
        }
        p = strtok_s(nullptr, delim, &ptr);
    }

    // Deal with HTTPS
    http_header->port = HTTP_PORT;
    if (!strcmp(http_header->host + strlen(http_header->host) - 3, "443")) {
        http_header->port = HTTPS_PORT;
    }
    return true;
}

HTTPHeader HTTP_Header_Parser::GetHeaderMessage() {
    return *http_header;
}