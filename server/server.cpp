#include <control.h>
#include <defs.h>
#include <iostream>
#include <map>
#include <mutex>
#include <player.h>
#include <server.hpp>
#include <sync.hpp>
#include <thread>
#include <vector>
#include <windows.h>
#include <winsock2.h>
#include <candy.hpp>

SOCKET sock;
using namespace std;

bool exit_flag = false;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    if (fdwCtrlType == CTRL_C_EVENT)
    {
        exit_flag = true;
        std::cout << "Socket closing..." << std::endl;
        closesocket(sock);
        return TRUE;
    }
    return FALSE;
}

static Control *control = Control::getInstance();
static CandyManager *candyMgr = CandyManager::getInstance();

vector<client> clients;
mutex clientsMtx;
vector<thread> recvThreads;
map<int, Player> players;
map<int, Arrow> arrowMap;

void sendThread()
{
    long long beginTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    while (true)
    {

        if (exit_flag)
        {
            break;
        }

        float currenttime = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() - beginTime) / 1000000000.0f;
        static int first = 0;
        if (first == 0)
        {
            control->oldTime = currenttime;
            first = 1;
        }
        control->dt = currenttime - control->oldTime;
        control->oldTime = currenttime;
        CandyManager *candyMgr = control->candyMgr;
        clientsMtx.lock();
        candyMgr->update(control->dt);

        for (auto &player : players)
        {
            SyncPackage *package = new PlayerSyncPackage(&player.second);
            for (int j = 0; j < clients.size(); j++)
            {
                int ret = package->send(clients[j].sock);
                if (ret == SOCKET_ERROR)
                {
                    std::cout << "Error: " << WSAGetLastError() << std::endl;
                    clients.erase(clients.begin() + j);
                    j--;
                    continue;
                }
                // std::cout << "send to " << j << std::endl;
            }
        }
        for (auto &arrow : arrowMap)
        {
            SyncPackage *package = new ArrowSyncPackage(&arrow.second);
            for (int j = 0; j < clients.size(); j++)
            {
                int ret = package->send(clients[j].sock);
                if (ret == SOCKET_ERROR)
                {
                    std::cout << "Error: " << WSAGetLastError() << std::endl;
                    clients.erase(clients.begin() + j);
                    j--;
                    continue;
                }
                // std::cout << "send to " << j << std::endl;
            }
        }
        clientsMtx.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
    }
}

void recvThread(int client_id, SOCKET client_sock)
{
    while (true)
    {
        if (exit_flag)
        {
            break;
        }
        SyncPackage *package;
        char buf[16];
        memset(buf, 0, sizeof(buf));
        int ret = recv(client_sock, buf, sizeof(buf), 0);
        SyncType type = *(SyncType *)(buf);
        if (type == Sync_Player)
            package = new PlayerSyncPackage();
        if (type == Sync_Arrow)
            package = new ArrowSyncPackage();
        if (type == Sync_Func)
            package = new FuncSyncPackage();
        package->type = *(SyncType *)(buf);
        package->timestamp = *(long long *)(buf + 4);
        package->size = *(int *)(buf + 12);
        // std::cout << "recv1 " << ret << std::endl;
        // std::cout << package.type << " " << package.timestamp << " " << package.size << std::endl;
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << endl;
            break;
        }
        package->data = new char[package->size];
        ret = recv(client_sock, package->data, package->size, 0);
        if (ret != package->size)
        {
            std::cout << "Error: " << WSAGetLastError() << std::endl;
            break;
        }
        // assert(ret == package.size);
        // std::cout << "recv2 " << ret << std::endl;
        // std::cout << package.data << std::endl;
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << endl;
            break;
        }
        // std::cout << "recv from " << client_id << std::endl;
        if (package->type == Sync_Player)
        {
            PlayerSyncPackage *player_package = (PlayerSyncPackage *)package;
            int id = player_package->getId();
            // std::cout << "sync player " << id << std::endl;
            player_package->update(&players[id]);
        }
        if (package->type == Sync_Arrow)
        {
            ArrowSyncPackage *arrow_package = (ArrowSyncPackage *)package;
            int id = arrow_package->getId();
            // std::cout << "sync arrow " << id << std::endl;
            arrow_package->update(&arrowMap[id]);
        }
        if (package->type == Sync_Func)
        {
            FuncSyncPackage *func_package = (FuncSyncPackage *)package;
            std::cout << "sync func" << std::endl;
            std::cout << func_package->getFuncType() << std::endl;
            if (func_package->getFuncType() == FUNC_CANDY_TOUCH)
            {
                FuncSyncPackage ret = move(candyMgr->touch(*func_package));
                if(ret.type != Sync_None)
                {
                    clientsMtx.lock();
                    for (int i = 0; i < clients.size(); i++)
                    {
                        ret.send(clients[i].sock);
                    }
                    clientsMtx.unlock();
                }
            }
            else
            {
                clientsMtx.lock();
                for (int i = 0; i < clients.size(); i++)
                {
                    if (clients[i].id != client_id)
                    {
                        func_package->send(clients[i].sock);
                        break;
                    }
                }
                clientsMtx.unlock();
            }
        }
    }
}

int main()
{
    current_player = -1;
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret == SOCKET_ERROR)
    {
        cout << "Error: " << WSAGetLastError() << endl;
        return 0;
    }
    sock = socket(AF_INET, SOCK_STREAM, 0);
    SetConsoleCtrlHandler(CtrlHandler, TRUE);
    if (sock == INVALID_SOCKET)
    {
        cout << "Error: " << WSAGetLastError() << endl;
        return 0;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);
    ret = bind(sock, (sockaddr *)&addr, sizeof(addr));
    if (ret == SOCKET_ERROR)
    {
        cout << "Error: " << WSAGetLastError() << endl;
        return 0;
    }
    ret = listen(sock, 10);
    if (ret == SOCKET_ERROR)
    {
        cout << "Error: " << WSAGetLastError() << endl;
        return 0;
    }
    thread send(sendThread);
    cout << "Server started" << endl;
    int client_id = 0;

    while (true)
    {
        if (exit_flag)
        {
            break;
        }
        sockaddr_in client_addr;
        int client_addr_size = sizeof(client_addr);
        SOCKET client_sock = accept(sock, (sockaddr *)&client_addr, &client_addr_size);
        if (client_sock == INVALID_SOCKET)
        {
            cout << "Error: " << WSAGetLastError() << endl;
            continue;
        }
        cout << "Client " << client_id << " connected" << endl;
        client client_t;
        client_t.id = client_id++;
        client_t.ip = client_addr.sin_addr.s_addr;
        client_t.port = client_addr.sin_port;
        client_t.sock = client_sock;
        clientsMtx.lock();
        clients.push_back(client_t);
        clientsMtx.unlock();
        recvThreads.push_back(thread(recvThread, client_t.id, client_t.sock));
    }
    for (int i = 0; i < clients.size(); i++)
    {
        closesocket(clients[i].sock);
    }
    WSACleanup();
    cout << "Socket closed" << endl;
    return 0;
}