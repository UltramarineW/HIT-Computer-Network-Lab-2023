#include "proxy_server.h"

ProxyServer::ProxyServer(int port)
{
    server_port = port;
}

bool ProxyServer::InitSocket()
{
    std::cout << "[Info] Starting init socket" << std::endl;
    // 套接字库
    WORD wVersionRequest;
    WSADATA wsaData;
    // 套接字加载错误提示
    int err;
    wVersionRequest = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequest, &wsaData);

    if (err != 0)
    {
        std::cerr << "[Error] Load winsock failed, error code: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        std::cerr << "[Error] Can't find right winsock version" << std::endl;
        WSACleanup();
        return false;
    }

    std::cout << "[Info] Init socket success" << std::endl;

    // 服务器端创建socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == server_socket)
    {
        std::cerr << "[Error] Create server socket failed, error code: " << WSAGetLastError() << std::endl;
        return false;
    }

    // 绑定并监听端口
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(server_port);
    server_sockaddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(server_socket, (SOCKADDR *)&server_sockaddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        std::cerr << "[Error] Bind socket failed" << std::endl;
        return false;
    }
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "[Error] Listen port " << server_port << " failed" << std::endl;
        return false;
    }
    return true;
}

void ProxyServer::Start()
{
    SOCKET accept_socket = INVALID_SOCKET;
    lp_proxy_param = new ProxyParam;

    while (true)
    {
        accept_socket = accept(server_socket, NULL, NULL);
        if (lp_proxy_param == NULL)
        {
            continue;
        }
        lp_proxy_param->client_socket = accept_socket;
        // TODO: Use modern C++ thread API
        hThread = (HANDLE)_beginthreadex(NULL, 0, &ProxyThread, (LPVOID)lp_proxy_param, 0, 0);
        CloseHandle(hThread);
        Sleep(200);
    }

    closesocket(server_socket);
    WSACleanup();
}

unsigned int __stdcall ProxyServer::ProxyThread(LPVOID lpParameter)
{
    char buffer[MAXSIZE];
    HTTPHeader *httpHeader = new HTTPHeader();
    char *CacheBuffer;
    ZeroMemory(buffer, MAXSIZE);
    SOCKADDR_IN client_addr;
    int length = sizeof(SOCKADDR_IN);
    int recv_size;
    int ret;
    recv_size = recv(((ProxyParam *)lpParameter)->client_socket, buffer, MAXSIZE, 0);
    if (recv_size <= 0)
    {
        goto error;
    }
    CacheBuffer = new char[recv_size + 1];
    ZeroMemory(CacheBuffer, recv_size + 1);
    memcpy(CacheBuffer, buffer, recv_size);
    ParseHTTPHead(CacheBuffer, httpHeader);
    delete CacheBuffer;
    

    // 错误处理
error:
    std::cout << "close socket" << std::endl;
    Sleep(200);
    closesocket(((ProxyParam *)lpParameter)->client_socket);
    closesocket(((ProxyParam *)lpParameter)->server_socket);
    // delete lpParameter;
    _endthreadex(0);
    return 0;
}

void ProxyServer::ParseHTTPHead(char *buffer, HTTPHeader *http_header)
{
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
    p = strtok_s(NULL, delim, &ptr);
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
        p = strtok_s(NULL, delim, &ptr);
    }
}

bool ProxyServer::ConnectToServer(SOCKET* server_socket, char *host) {
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(HTTP_PORT);
    HOSTENT *hostent = gethostbyname(host);
    if (!hostent) {
        return false;
    }
    in_addr in_addr_host = *((in_addr*) *hostent->h_addr_list);
    server_addr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(in_addr_host));
    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket == INVALID_SOCKET) {
        return false;
    }
    if (connect(*server_socket, (SOCKADDR *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(*server_socket);
        return false;
    }
    return true;
}