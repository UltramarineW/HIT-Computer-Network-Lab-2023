#include "proxy_server.h"

ProxyServer::ProxyServer(int port) : pool(FLAGS_thread_nums), server_port (port) {}

bool ProxyServer::InitSocket() {
    std::cout << "[Info] Starting init socket" << std::endl;
    // 套接字库
    WORD wVersionRequest;
    WSADATA wsaData;
    // 套接字加载错误提示
    int err;
    wVersionRequest = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequest, &wsaData);

    if (err != 0) {
        std::cerr << "[Error] Load winsock failed, error code: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        std::cerr << "[Error] Can't find right winsock version" << std::endl;
        WSACleanup();
        return false;
    }

    std::cout << "[Info] Init socket success" << std::endl;

    // 服务器端创建socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == server_socket) {
        std::cerr << "[Error] Create server socket failed, error code: " << WSAGetLastError() << std::endl;
        return false;
    }

    // 绑定并监听端口
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(server_port);
    server_sockaddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(server_socket, (SOCKADDR *) &server_sockaddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        std::cerr << "[Error] Bind socket failed" << std::endl;
        return false;
    }
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[Error] Listen port " << server_port << " failed" << std::endl;
        return false;
    }
    return true;
}

void ProxyServer::Start() {
    if (!InitSocket()) {
        std::cerr << "[Error] Server init socket failed" << std::endl;
    }

    SOCKET accept_socket = INVALID_SOCKET;
    lp_proxy_param = new ProxyParam;

    while (true) {
        accept_socket = accept(server_socket, nullptr, nullptr);
        if (accept_socket == SOCKET_ERROR) {
            std::cerr << "[Error] Accept socket error" << std::endl;
        }
        if (lp_proxy_param == nullptr) {
            continue;
        }
        lp_proxy_param->client_socket = accept_socket;
        // TODO: Use modern C++ thread API

        ProxyTask proxy_task(lp_proxy_param);

//        hThread = (HANDLE) _beginthreadex(nullptr, 0, &ProxyThread, (LPVOID) lp_proxy_param, 0, 0);

//        pool.add_task();
//        CloseHandle(hThread);
        Sleep(200);
    }

    closesocket(server_socket);
    WSACleanup();
}


unsigned int __stdcall ProxyServer::ProxyThread(LPVOID lpParameter) {
    int recv_size;
    int ret;
    int length = sizeof(SOCKADDR_IN);
    char buffer[MAXSIZE];
    char *CacheBuffer;
    ZeroMemory(buffer, MAXSIZE);
    SOCKADDR_IN client_addr;
    HTTP_Header_Parser parser{};

    recv_size = recv(((ProxyParam *) lpParameter)->client_socket, buffer, MAXSIZE, 0);
    if (recv_size <= 0) {
        Sleep(200);
        closesocket(((ProxyParam *) lpParameter)->client_socket);
        closesocket(((ProxyParam *) lpParameter)->server_socket);
        // delete lpParameter;
        _endthreadex(0);
        return -1;
    }
    CacheBuffer = new char[recv_size + 1];
    ZeroMemory(CacheBuffer, recv_size + 1);
    memcpy(CacheBuffer, buffer, recv_size);
    parser.Parse(CacheBuffer);
    delete CacheBuffer;

    std::cout << "close socket" << std::endl;
    Sleep(200);
    closesocket(((ProxyParam *) lpParameter)->client_socket);
    closesocket(((ProxyParam *) lpParameter)->server_socket);
    // delete lpParameter;
    _endthreadex(0);
    return 0;
}

bool ProxyServer::ConnectToServer(SOCKET *server_socket, char *host) {
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(HTTP_PORT);
    HOSTENT *hostent = gethostbyname(host);
    if (!hostent) {
        return false;
    }
    in_addr in_addr_host = *((in_addr *) *hostent->h_addr_list);
    server_addr.sin_addr.S_un.S_addr = inet_addr(inet_ntoa(in_addr_host));
    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket == INVALID_SOCKET) {
        return false;
    }
    if (connect(*server_socket, (SOCKADDR *) &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        closesocket(*server_socket);
        return false;
    }
    return true;
}