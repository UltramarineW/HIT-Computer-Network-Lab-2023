#include "http_header_parser.h"

HTTP_Header_Parser::HTTP_Header_Parser() {
    http_header = std::make_unique<HTTPHeader>();
}

bool HTTP_Header_Parser::Parse(char *buffer) {
    char *p;
    char *ptr;
    const char *delim = "\r\n";
    p = strtok_s(buffer, delim, &ptr);
    std::cout << p << std::endl;
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

    std::cout << http_header->url << std::endl;
    p = strtok_s(nullptr, delim, &ptr);
    while (p)
    {
        switch (p[0])
        {
        case 'H':
            memcpy(http_header->host, &p[6], strlen(p) - 6);
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
    return true;
}

HTTPHeader HTTP_Header_Parser::GetHeaderMessage() {
    return *http_header;
}