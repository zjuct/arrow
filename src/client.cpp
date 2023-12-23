#include <control.h>
#include <defs.h>
#include <iostream>
#include <mutex>
#include <player.h>
#include <sync.hpp>
#include <thread>
#include <vector>
#include <windows.h>
#include <winsock2.h>

// void clientInit()
// {
//     WSADATA wsaData;
//     WSAStartup(MAKEWORD(2, 2), &wsaData);
//     sock = socket(AF_INET, SOCK_STREAM, 0);
// }

static Control *control = Control::getInstance();

SOCKET sock;

extern int current_player;

extern int init;

void recvThread()
{
    std::cout << "recvThread" << std::endl;
    while (true)
    {
        SyncPackage package;
        package.recv(sock);
        switch (package.type)
        {
        case Sync_Player:
        {
            PlayerSyncPackage *player_package = (PlayerSyncPackage *)&package;
            player_package->update(&control->players[player_package->getId()]);
            std::cout<<"id: "<<player_package->getId()<<std::endl;
            break;
        }
        default:
            break;
        }
    }
}

int clientThread()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    // std::string ip = "10.162.69.158";
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

    while (!init)
        std::cout << "init" << std::endl;
    std::thread recv(recvThread);
    while (!glfwWindowShouldClose(control->window))
    {
        SyncPackage *package = new PlayerSyncPackage(&control->players[current_player]);
        package->send(sock);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }
    return 0;
}