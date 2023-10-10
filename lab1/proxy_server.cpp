#include "proxy_server.h"

 ProxyServer::ProxyServer(int port) : pool_(FLAGS_thread_nums), server_port_(port), server_socket_(0) {
     filter_ptr_ = std::make_shared<HttpFilter>(
             R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab1\list\website_white_list.txt)",
             R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab1\list\website_black_list.txt)",R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab1\list\user_white_list.txt)",
             R"(E:\HIT_Project\HIT-Computer-Network-Lab-2023\lab1\list\user_black_list.txt)");
}

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
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (INVALID_SOCKET == server_socket_) {
        std::cerr << "[Error] Create server socket failed, error code: " << WSAGetLastError() << std::endl;
        return false;
    }

    // 绑定并监听端口
    server_sockaddr_.sin_family = AF_INET;
    server_sockaddr_.sin_port = htons(server_port_);
    server_sockaddr_.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(server_socket_, (sockaddr *) &server_sockaddr_, sizeof(sockaddr)) == SOCKET_ERROR) {
        std::cerr << "[Error] Bind socket failed" << std::endl;
        return false;
    }
    if (listen(server_socket_, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[Error] Listen port " << server_port_ << " failed" << std::endl;
        return false;
    }
    return true;
}

void ProxyServer::Start() {
    // Init proxy server socket (ready to connect with client)
    if (!InitSocket()) {
        std::cerr << "[Error] Server init socket failed" << std::endl;
    }

    SOCKET accept_socket = INVALID_SOCKET;
    std::unique_ptr<ProxyParam> lp_proxy_param = std::make_unique<ProxyParam>();

    // start thread pool_
    pool_.start();

    while (true) {
        std::cout << std::endl;
        std::cout << "[Info] Starting accept socket" << std::endl;
        accept_socket = accept(server_socket_, nullptr, nullptr);
        if (accept_socket == SOCKET_ERROR) {
            std::cerr << "[Error] Accept socket error" << std::endl;
        }
        lp_proxy_param->client_socket = accept_socket;

        std::cout << "Proxy task add to list, client socket: " << lp_proxy_param -> client_socket << std::endl;

        // add task to the thread pool
        pool_.add_task([&lp_proxy_param, this]{
            ProxyTask proxy_task(*lp_proxy_param, filter_ptr_);
            proxy_task.Run();
        });

        Sleep(200);
    }

    closesocket(server_socket_);
    WSACleanup();
}