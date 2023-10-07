#include "proxy_server.h"

 ProxyServer::ProxyServer(int port) : pool(FLAGS_thread_nums), server_port(port) {}

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
    if (bind(server_socket, (sockaddr *) &server_sockaddr, sizeof(sockaddr)) == SOCKET_ERROR) {
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
    // Init proxy server socket (ready to connect with client)
    if (!InitSocket()) {
        std::cerr << "[Error] Server init socket failed" << std::endl;
    }

    SOCKET accept_socket = INVALID_SOCKET;
    std::unique_ptr<ProxyParam> lp_proxy_param = std::make_unique<ProxyParam>();
    // start thread pool
    pool.start();

    while (true) {
        std::cout << std::endl;
        std::cout << "[Info] Starting accept socket" << std::endl;
        accept_socket = accept(server_socket, nullptr, nullptr);
        if (accept_socket == SOCKET_ERROR) {
            std::cerr << "[Error] Accept socket error" << std::endl;
        }
        if (lp_proxy_param == nullptr) {
            continue;
        }
        lp_proxy_param->client_socket = accept_socket;

        // add task to the threadpool
        pool.add_task([&lp_proxy_param]{
            ProxyTask proxy_task(lp_proxy_param.get());
            proxy_task.Run();
        });

        Sleep(200);
    }

    closesocket(server_socket);
    WSACleanup();
}
