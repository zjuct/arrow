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

vector<client> clients;
mutex clientsMtx;
vector<thread> recvThreads;


void sendThread()
{
    while (true)
    {
        if (exit_flag)
        {
            break;
        }
        clientsMtx.lock();
        for (int i = 0; i < clients.size(); i++)
        {
            SyncPackage *package = new PlayerSyncPackage(&control->players[clients[i].id]);
            for(int j = 0; j < clients.size(); j++)
            {
                if (i == j)
                {
                    continue;
                }
                package->send(clients[j].sock);
            }
        }
        clientsMtx.unlock();
        Sleep(10);
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
        SyncPackage package;
        char buf[16];
        memset(buf, 0, sizeof(buf));
        int ret = recv(client_sock, buf, sizeof(buf), 0);
        package.type = *(SyncType*)(buf);
        package.timestamp = *(long long*)(buf + 4);
        package.size = *(int*)(buf + 12);
        std::cout << "recv1 " << ret << std::endl;
        std::cout << package.type << " " << package.timestamp << " " << package.size << std::endl;
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << endl;
            break;
        }
        package.data = new char[package.size];
        ret = recv(client_sock, package.data, package.size, 0);
        std::cout << "recv2 " << ret << std::endl;
        std::cout << package.data << std::endl;
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << endl;
            break;
        }
        std::cout<<"recv from "<<client_id<<std::endl;
        if (package.type == Sync_Player)
        {
            PlayerSyncPackage *player_package = (PlayerSyncPackage *)&package;
            player_package->update(&control->players[client_id]);
        }
    }
}

int main()
{
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
        control->players.push_back(Player());
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