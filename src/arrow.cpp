#include "arrow.hpp"
#include "defs.h"
#include "control.h"

static Control *control = Control::getInstance();
static ArrowManager *arrowMgr = ArrowManager::getInstance();

Arrow::Arrow(Object arrow_normal, Object arrow_laser, Object arrow_ground_spike)
{
    this->arrow_normal = arrow_normal;
    this->arrow_laser = arrow_laser;
    this->arrow_ground_spike = arrow_ground_spike;
}

void Arrow::draw()
{
    if (state == ARROW_DISAPPEAR | state == ARROW_NONE)
        return;
    updateModel();
    // std::cout << "state: " << state << std::endl;
    // std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    switch (type)
    {
    case ARROW_NORMAL:
        arrow_normal.draw();
        break;
    case ARROW_LASER:
        arrow_laser.draw();
        break;
    case ARROW_GROUND_SPIKE:
        arrow_ground_spike.draw();
        break;
    }
}

void Arrow::update(float dt)
{
    if (state == ARROW_FLY)
    {
        if (type == ARROW_NORMAL)
        {
            glm::vec3 g = glm::vec3(0.0f, -GRAVITY, 0.0f);
            glm::vec3 delta = dir * speed + g * dt * weight / WIND_RESISTANCE;
            pos += delta;
            dir = glm::normalize(delta);
        }
        if (pos.y <= FLOOR_Y - 10.0f)
        {
            state = ARROW_DISAPPEAR;
        }
    }
    if (state == ARROW_LOADING)
    {
        loadTime -= dt;
        if (loadTime <= 0.0f)
        {
            state = ARROW_HOLD;
        }
    }
    if (state == ARROW_HIT_WALL | state == ARROW_HIT_PLAYER | state == ARROW_ON_FLOOR)
    {
        disappearTime -= dt;
        if (disappearTime <= 0.0f)
        {
            state = ARROW_DISAPPEAR;
        }
    }
    // if(state == ARROW_SPIKE_LIVE)
    // {
    //     liveTime -= dt;
    //     if(liveTime <= 0.0f)
    //     {
    //         state = ARROW_DISAPPEAR;
    //     }
    // }
    if (state > ARROW_HOLD)
    {
        liveTime -= dt;
        if (liveTime <= 0.0f)
        {
            state = ARROW_DISAPPEAR;
        }
    }
}

void Arrow::update(glm::vec3 pos, glm::vec3 dir)
{
    if (state == ARROW_HOLD | state == ARROW_HIT_PLAYER | state == ARROW_LOADING)
    {
        this->pos = pos;
        this->dir = dir;
    }
}

bool Arrow::fire(glm::vec3 pos, glm::vec3 dir, float pressTime)
{
    if (state != ARROW_HOLD)
        return false;
    this->pos = pos;
    this->dir = dir;
    this->state = ARROW_FLY;
    pressTime -= 0.2f;
    if (pressTime < 0.0f)
        pressTime = 0.0f;
    float strength = strengthMin + (strengthMax - strengthMin) * pressTime / strengthTime;
    if (strength > strengthMax)
        strength = strengthMax;
    this->speed *= strength;
    return true;
}

void Arrow::updateModel()
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    //旋转到dir方向
    glm::vec3 axis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), dir);
    float angle = glm::acos(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), dir));
    model = glm::rotate(model, angle, axis);
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    switch (type)
    {
    case ARROW_NORMAL:
        arrow_normal.setModel(model);
        break;
    case ARROW_LASER:
        arrow_laser.setModel(model);
        break;
    case ARROW_GROUND_SPIKE:
        arrow_ground_spike.setModel(model);
        break;
    }
}

static ArrowManager *instance = nullptr;
ArrowManager *ArrowManager::getInstance()
{
    static ArrowManager instance;
    return &instance;
}

void ArrowManager::init(const char *objfile)
{
    obj = Scene::LoadObj(objfile);
    std::vector<Mesh> &meshes = obj->getMesh();
    // .obj中定义的顺序必须为arrow_normal, arrow_laser, arrow_ground_spike
    arrow_normal = Object(OBJECT_MESH, &meshes[4], player_shader);
    arrow_laser = Object(OBJECT_MESH, &meshes[1], player_shader);
    arrow_ground_spike = Object(OBJECT_MESH, &meshes[2], player_shader);
}

void ArrowManager::draw()
{
    for (auto &[_, arrow] : arrows)
    {
        arrow.draw();
    }
}

void ArrowManager::update(float dt)
{
    // std::cout << "size:" << arrows.size() << std::endl;
    for (auto &[_, arrow] : arrows)
    {
        arrow.update(dt);
    }
    for (auto it = arrows.begin(); it != arrows.end();)
    {
        if (it->second.state == ARROW_DISAPPEAR)
        {
            it = arrows.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void ArrowManager::bindArrow(int playerId, glm::vec3 pos, glm::vec3 dir, ArrowType type, float speed, float scale, float weight, float loadTime)
{
    Arrow arrow = Arrow(arrowMgr->arrow_normal, arrowMgr->arrow_laser, arrowMgr->arrow_ground_spike);
    arrow.type = type;
    arrow.scale = scale;
    arrow.speed = speed;
    arrow.weight = weight;
    arrow.loadTime = loadTime;
    arrow.state = ARROW_NONE;
    arrow.update(pos, dir);
    arrows[++arrowCnt] = arrow;
    arrowSetting[playerId] = arrowCnt;
    load(playerId);
}

void ArrowManager::load(int playerId)
{
    if (arrowSetting.find(playerId) == arrowSetting.end())
        assert(false);
    arrows[++arrowCnt] = arrows[arrowSetting[playerId]];
    arrowMap[playerId] = arrowCnt;
    arrows[arrowCnt].state = ARROW_LOADING;
}

void ArrowManager::fire(int playerId, glm::vec3 pos, glm::vec3 dir, float pressTime)
{
    if (arrowMap.find(playerId) == arrowMap.end())
        return;
    bool t = arrows[arrowMap[playerId]].fire(pos, dir, playerId == PLAYER_ID ? pressTime : 1.0f);
    std::cout << "fire time: " << pressTime << std::endl;
    if (t)
        load(playerId);
}

void ArrowManager::updateArrow(int playerId, glm::vec3 pos, glm::vec3 dir)
{
    if (arrowMap.find(playerId) == arrowMap.end())
        return;
    arrows[arrowMap[playerId]].update(pos, dir);
}

void ArrowManager::deleteArrow(int playerId)
{
    if (arrowSetting.find(playerId) != arrowSetting.end())
    {
        arrows.erase(arrowSetting[playerId]);
        arrowSetting.erase(playerId);
    }
    if (arrowMap.find(playerId) != arrowMap.end())
    {
        arrows.erase(arrowMap[playerId]);
        arrowMap.erase(playerId);
    }
}