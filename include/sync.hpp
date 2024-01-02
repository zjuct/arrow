#ifndef _SYNC_HPP
#define _SYNC_HPP

#include <candy.hpp>
#include <chrono>
#include <player.h>
#include <winsock2.h>

enum SyncType
{
    Sync_None,
    Sync_Player,
    Sync_Candy,
    Sync_Arrow,
    Sync_Event,
    Sync_Func,
    Sync_Num,
};

enum FuncType
{
    FUNC_NONE,
    FUNC_ARROW_BIND,
    FUNC_ARROW_LOAD,
    FUNC_ARROW_FIRE,
    FUNC_ARROW_UPDATE,
    FUNC_PLAYER_REBIRTH,
    FUNC_CANDY_GENERATE,
    FUNC_CANDY_TOUCH,
    FUNC_CANDY_EATEN,
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
        if (data && size)
            delete[] data;
    }

    SyncPackage() : type(Sync_None), timestamp(0), size(0), data(nullptr) {}
    SyncPackage(SyncPackage &package)
    {
        type = package.type;
        timestamp = package.timestamp;
        size = package.size;
        data = new char[size];
        memcpy(data, package.data, size);
    }
    SyncPackage(SyncPackage &&package)
    {
        type = package.type;
        timestamp = package.timestamp;
        size = package.size;
        data = package.data;
        package.data = nullptr;
        package.size = 0;
    }

    int send(SOCKET sock)
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
        return ret;
    }

    // void broadcast(std::vector<client> socks)
    // {
    //     for(auto sock : socks)
    //         send(sock);
    // }

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
    PlayerSyncPackage() {}
    PlayerSyncPackage(SyncPackage &package) : SyncPackage(package) {}
    PlayerSyncPackage(Player *player);
    void update(Player *player);
    int getId();
};

class ArrowSyncPackage : public SyncPackage
{
public:
    ArrowSyncPackage() {}
    ArrowSyncPackage(SyncPackage &package) : SyncPackage(package) {}
    ArrowSyncPackage(Arrow *arrow);
    void update(Arrow *arrow);
    int getId();
};

class CandySyncPackage : public SyncPackage
{
public:
    // CandySyncPackage(){}
    // CandySyncPackage(Candy *candy);
    // void update(Candy *candy);
    // int getId();
};

class FuncSyncPackage : public SyncPackage
{
public:
    FuncSyncPackage() {}
    FuncSyncPackage(SyncPackage &package) : SyncPackage(package) {}
    template <typename T, typename... args>
    FuncSyncPackage(FuncType funcType, T *param, args... arg)
    {
        type = Sync_Func;
        timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        size = 0;
        getSize(&funcType, param, arg...);
        data = new char[size];
        init(0, &funcType, param, arg...);
    }

    template <typename T, typename... args>
    void init(int offset, T *param, args... arg)
    {
        memcpy(data + offset, param, sizeof(T));
        init(offset + sizeof(T), arg...);
    }

    template <typename T, typename... args>
    void init(int offset, T *param)
    {
        memcpy(data + offset, param, sizeof(T));
    }

    template <typename T, typename... args>
    void getSize(T *param, args... arg)
    {
        size += sizeof(T);
        getSize(arg...);
    }

    template <typename T, typename... args>
    void getSize(T *param)
    {
        size += sizeof(T);
    }

    template <typename T, typename... args>
    void get(T *param, args... arg)
    {
        getfrom(sizeof(FuncType), param, arg...);
    }

    template <typename T, typename... args>
    void getfrom(int offset, T *param, args... arg)
    {
        memcpy(param, data + offset, sizeof(T));
        getfrom(offset + sizeof(T), arg...);
    }

    template <typename T, typename... args>
    void getfrom(int offset, T *param)
    {
        memcpy(param, data + offset, sizeof(T));
    }

    FuncType getFuncType()
    {
        FuncType funcType;
        memcpy(&funcType, data, sizeof(FuncType));
        return funcType;
    }
};

#endif // _SYNC_HPP