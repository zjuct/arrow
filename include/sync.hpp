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
        int ret = ::send(sock, (char *)&type, sizeof(SyncType), 0);
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << std::endl;
        }
        ret = ::send(sock, (char *)&timestamp, sizeof(long long), 0);
        if (ret == SOCKET_ERROR)
        {
            std::cout << "Error: " << WSAGetLastError() << std::endl;
        }
        ret = ::send(sock, (char *)&size, sizeof(int), 0);
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
};


#endif // _SYNC_HPP