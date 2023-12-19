#include "candy.hpp"
#include "defs.h"
#include "control.h"

static Control *control = Control::getInstance();
static CandyManager *candyMgr = CandyManager::getInstance();

void Candy::draw()
{
    if (type == CANDY_NONE)
        return;
    updateModel();
    candy.draw();
}

void Candy::updateModel()
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::rotate(model, rotateDir, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(scale, scale, scale));
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
    if(rotateDir > 360.0f)
        rotateDir -= 360.0f;
    
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
    std::vector<Mesh> &meshes = obj->getMesh();
    // .obj中定义的顺序必须为arrow_normal, arrow_laser, arrow_ground_spike
    model = Object(OBJECT_MESH, &meshes[4], player_shader);
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
    candies.push_back(Candy(model, pos, 0.0f, 1.0f, type));
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
