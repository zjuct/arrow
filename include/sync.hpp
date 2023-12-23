#ifndef _SYNC_HPP
#define _SYNC_HPP

#include <player.h>
#include <winsock2.h>
#include <chrono>

enum SyncType
{
    Sync_Player,
    Sync_Candy,
    Sync_Arrow,
    Sync_Num,
};

class SyncPackage
{
public:
    SyncType type;
    long long timestamp;
    int size;
    char *data;

    virtual ~SyncPackage()
    {
        if (data)
            delete[] data;
    }

    void send(SOCKET sock)
    {
        char sendbuf[16];
        memcpy(sendbuf, (char *)&type, sizeof(SyncType));
        memcpy(sendbuf + sizeof(SyncType), (char *)&timestamp, sizeof(long long));
        memcpy(sendbuf + sizeof(SyncType) + sizeof(long long), (char *)&size, sizeof(int));
        int ret = ::send(sock, sendbuf, 16, 0);
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << std::endl;
        }
        ret = ::send(sock, data, size, 0);
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << std::endl;
        }
    }

    void recv(SOCKET sock)
    {
        char buf[16];
        memset(buf, 0, sizeof(buf));
        int ret = ::recv(sock, buf, sizeof(buf), 0);
        type = *(SyncType *)(buf);
        timestamp = *(long long *)(buf + 4);
        size = *(int *)(buf + 12);
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << std::endl;
            return;
        }
        data = new char[size];
        ret = ::recv(sock, data, size, 0);
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << std::endl;
            return;
        }
    }
};

struct client
{
    int id;
    int ip;
    int port;
    SOCKET sock;
};

class PlayerSyncPackage : public SyncPackage
{
public:
    PlayerSyncPackage(){}
    PlayerSyncPackage(Player *player);
    void update(Player *player);
    int getId();
};


#endif // _SYNC_HPP