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

#define CLIENT (1)

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
extern std::mutex updateMutex;

void recvThread()
{
    std::cout << "recvThread" << std::endl;
    while (true)
    {
        SyncPackage package;
        package.recv(sock);
        updateMutex.lock();
        switch (package.type)
        {
        case Sync_Player:
        {
            PlayerSyncPackage *player_package = new PlayerSyncPackage(package);
            if (player_package->getId() == current_player)
                break;
            player_package->update(&control->players[player_package->getId()]);
            // std::cout << "id: " << player_package->getId() << std::endl;
            break;
        }
        case Sync_Arrow:
        {
            ArrowSyncPackage *arrow_package = new ArrowSyncPackage(package);
            if (arrow_package->getId() == current_player)
                break;
            int player_id = arrow_package->getId();
            if (!control->arrowMgr->arrowMap.count(player_id))
            {
                control->arrowMgr->bindArrow(player_id);
                arrow_package->update(&control->arrowMgr->arrows[control->arrowMgr->arrowSetting[player_id]]);
                control->arrowMgr->load(player_id);
            }
            arrow_package->update(&control->arrowMgr->arrows[control->arrowMgr->arrowMap[player_id]]);
            // std::cout << "id: " << arrow_package->getId() << std::endl;
            break;
        }
        case Sync_Func:
        {
            // FUNC_ARROW_BIND,
            // FUNC_ARROW_LOAD,
            // FUNC_ARROW_FIRE,
            // FUNC_ARROW_UPDATE,
            FuncSyncPackage *func_package = new FuncSyncPackage(package);
            FuncType funcType = func_package->getFuncType();
            switch (funcType)
            {
            case FUNC_ARROW_BIND:
                control->arrowMgr->bindArrow(*func_package);
                break;
            case FUNC_ARROW_LOAD:
                control->arrowMgr->load(*func_package);
                break;
            case FUNC_ARROW_FIRE:
                control->arrowMgr->fire(*func_package);
                break;
            case FUNC_ARROW_UPDATE:
                // control->arrowMgr->updateArrow(*func_package);
                break;
            case FUNC_PLAYER_REBIRTH:
            {
                int id = -1;
                func_package->get(&id);
                control->players[id].rebirth();
                break;
            }
            }
        }
        default:
            break;
        }
        updateMutex.unlock();
    }
}

void clientInit()
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
        return;
    }

    std::cout << "Connected to server" << std::endl;
}

int clientThread()
{
    while (!init)
        std::cout << "init" << std::endl;
    std::thread recv(recvThread);
    while (!glfwWindowShouldClose(control->window))
    {
        SyncPackage *package = new PlayerSyncPackage(&control->players[current_player]);
        package->send(sock);
        package = new ArrowSyncPackage(&control->arrowMgr->arrows[control->arrowMgr->arrowMap[current_player]]);
        package->send(sock);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
    }
    return 0;
}