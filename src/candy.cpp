#include "candy.hpp"
#include "defs.h"
#include "control.h"

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

void Candy::draw()
{
    if (type == CANDY_NONE)
        return;
    updateModel();
    // printMat4(candy->getGModelNoscale());
    candy.draw();
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
    if (liveTime <= 0.0f)
    {
        type = CANDY_DISAPPEAR;
    }
    rotateDir += rotateSpeed * dt;
    if (rotateDir > 360.0f)
        rotateDir -= 360.0f;
    scale = 1.0f + sin(liveTime) * 0.2f;
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

void CandyManager::draw()
{
    for (auto &candy : candies)
    {
        candy.draw();
    }
}

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
            it = candies.erase(it);
        }
        else
        {
            it++;
        }
    }
    generateTime -= dt;
    if (generateTime <= 0.0f)
    {
        generateCandy();
        generateTime = 1.0f;
    }
}

void CandyManager::generateCandy(glm::vec3 pos, CandyType type)
{
    candies.push_back(Candy(model, pos, type, 0.0f, 1.0f));
}

void CandyManager::generateCandy()
{
    float x = rand() % 1000 / 1000.0f * MAP_SIZE - MAP_SIZE / 2.0f;
    float z = rand() % 1000 / 1000.0f * MAP_SIZE - MAP_SIZE / 2.0f;
    // std::cout << x << " " << z << std::endl;
    float y = FLOOR_Y + 0.2f;
    glm::vec3 pos = glm::vec3(x, y, z);
    CandyType type = (CandyType)(rand() % CANDY_TYPE_NUM);
    generateCandy(pos, type);
}
