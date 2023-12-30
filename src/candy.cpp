#include "candy.hpp"
#include "control.h"
#include "defs.h"
#include <ray.h>
#include <sync.hpp>
#include <vector>
#include <winsock2.h>

static Control *control = Control::getInstance();
static CandyManager *candyMgr = CandyManager::getInstance();

// #include <iostream>

// void printMat4(const glm::mat4& mat) {
//     for(int i = 0; i < 4; i++) {
//         for(int j = 0; j < 4; j++) {
//             std::cout << mat[i][j] << " ";
//         }
//         std::cout << std::endl;
//     }
//     std::cout << "-------------" << std::endl;
// }

void Candy::draw(Shader* shader)
{
    if (type == CANDY_NONE)
        return;
    // printMat4(candy->getGModelNoscale());
    candy.draw(shader);
}

void Candy::updateModel()
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::rotate(model, rotateDir, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    candy.setModel_noscale(model);
    candy.setModel(model);
}

void Candy::update(float dt)
{
    if (type == CANDY_NONE | type == CANDY_DISAPPEAR)
        return;
    liveTime -= dt;
    if (liveTime <= 0.0f && type != CANDY_DISAPPEARING)
    {
        type = CANDY_DISAPPEARING;
        liveTime = -0.3f;
    }
    if (liveTime <= -1.0f)
    {
        type = CANDY_DISAPPEAR;
    }
    rotateDir += rotateSpeed * dt;
    if (rotateDir > 360.0f)
        rotateDir -= 360.0f;
    scale = 1.0f + sin(liveTime) * 0.2f;
    if (type == CANDY_DISAPPEARING)
    {
        scale = 1.0f / (-liveTime + 0.7f);
    }
}

void Candy::hit()
{
    liveTime = 0;
}

CandyManager *CandyManager::getInstance()
{
    static CandyManager candyMgr;
    return &candyMgr;
}

void CandyManager::init(const char *objfile)
{
    obj = Scene::LoadObj(objfile);
    model = Object(OBJECT_NONE, nullptr, nullptr, glm::mat4(1.0),
                   nullptr, false);
    for (auto &mesh : obj->getMesh())
    {
        Object *node = new Object(OBJECT_MESH, &mesh, player_shader, glm::mat4(1.0f), &model);
    }
}

void CandyManager::draw(Shader* shader)
{
    for (auto &candy : candies)
    {
        candy.draw(shader);
    }
}

void CandyManager::updateModel()
{
    for (auto &candy : candies)
    {
        candy.updateModel();
    }
}

#ifdef SERVER
extern std::vector<client> clients;
#endif

extern SOCKET sock;

void CandyManager::update(float dt)
{
    for (auto &candy : candies)
    {
        candy.update(dt);
    }
    for (auto it = candies.begin(); it != candies.end();)
    {
        if (it->type == CANDY_DISAPPEAR)
        {
            // std::cout<<"disappear"<<std::endl;
            it = candies.erase(it);
        }
        else
        {
            it++;
        }
    }
#ifdef SERVER  
    std::cout<<"size:"<<candies.size()<<std::endl;
    generateTime -= dt;
    if (generateTime <= 0.0f)
    {
        generateCandy();
        generateTime = generateTimeMax;
    }
#endif
}

void CandyManager::generateCandy(glm::vec3 pos, CandyType type)
{
    candies.emplace_back(model, pos, type, 0.0f, 1.0f);
}

void CandyManager::generateCandy(FuncSyncPackage &funcSyncPackage)
{
    glm::vec3 pos;
    CandyType type;
    funcSyncPackage.get(&pos, &type);
    candies.emplace_back(model, pos, type, 0.0f, 1.0f);
}

void CandyManager::generateCandy()
{
    float x = rand() % 1000 / 1000.0f * MAP_SIZE - MAP_SIZE / 2.0f;
    float z = rand() % 1000 / 1000.0f * MAP_SIZE - MAP_SIZE / 2.0f;
    // std::cout << x << " " << z << std::endl;
    Ray ray(glm::vec3(x, 100.0f, z), glm::vec3(0.0f, -1.0f, 0.0f));
    IntersectPoint intersectPoint = ray.intersectWith(control->ground.getModel());
    float y = intersectPoint.p.y + 0.2f;
    glm::vec3 pos = glm::vec3(x, y, z);
    CandyType type = (CandyType)(CANDY_NORMAL);
    generateCandy(pos, type);
#ifdef SERVER
    FuncSyncPackage funcSyncPackage = FuncSyncPackage(FUNC_CANDY_GENERATE, &pos, &type);
    for(int i = 0; i < clients.size(); i++) {
        funcSyncPackage.send(clients[i].sock);
    }
#endif
}

void CandyManager::eat(Player &player)
{
    for (auto it = candies.begin(); it != candies.end(); it++)
    {
        if (glm::length(it->pos - player.position) < 1.0f && it->type < CANDY_DISAPPEARING)
        {
            if (it->candy.intersectWith(player.getBody()) || it->candy.intersectWith(player.getHead()))
            {
                // it->liveTime = 0.0f;
                // player.getCandy(it->type);
                // it->type = CANDY_DISAPPEARING;
                FuncSyncPackage funcSyncPackage = FuncSyncPackage(FUNC_CANDY_TOUCH, &player.id, &it->id);
                funcSyncPackage.send(sock);
            }
        }
    }
}

int CandyManager::touch(FuncSyncPackage &funcSyncPackage)
{
    int player_id;
    int candy_id;
    funcSyncPackage.get(&player_id, &candy_id);
    for(auto it = candies.begin(); it != candies.end(); it++) {
        if(it->id == candy_id) {
            it->liveTime = 0.0f;
            control->players[player_id].getCandy(it->type);
            it->type = CANDY_DISAPPEARING;
            break;
        }
    }
    
    return 0;
}

int Candy::idCnt = 0;