#include <winsock2.h>
#include <sync.hpp>
#include <iostream>
#include <windows.h>
#include <defs.h>
#include <vector>
#include <mutex>
#include <player.h>
#include <control.h>    
#include <thread>

// void clientInit()
// {
//     WSADATA wsaData;
//     WSAStartup(MAKEWORD(2, 2), &wsaData);
//     sock = socket(AF_INET, SOCK_STREAM, 0);
// }

static Control *control = Control::getInstance();

SOCKET sock;

extern int current_player;

int clientThread()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
    addr.sin_family = AF_INET;
    std::string ip = "127.0.0.1";
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    int port = SERVER_PORT;
    addr.sin_port = htons(port);
    int ret = connect(sock, (sockaddr *)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR)
    {
        std::cout << "Error: " << WSAGetLastError() << std::endl;
        return 0;
    }

    std::cout << "Connected to server" << std::endl;

    while (!glfwWindowShouldClose(control->window))
    {
        SyncPackage *package = new PlayerSyncPackage(&control->players[current_player]);
        package->send(sock);
    }
    return 0;
}