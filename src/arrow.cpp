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
    if (state == ARROW_LOADING)
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
    Object &arrow = [&]() -> Object &
    {
        switch (type)
        {
        case ARROW_NORMAL:
            return arrow_normal;
        case ARROW_LASER:
            return arrow_laser;
        case ARROW_GROUND_SPIKE:
            return arrow_ground_spike;
        }
        return arrow_normal;
    }();
    if (state == ARROW_FLY)
    {
        std::cout << "pos_update: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
        if (pos.y <= FLOOR_Y - 100.f)
        {
            state = ARROW_ON_FLOOR;
        }
        int checkTime = 3;
        float l = 0.0f, r = dt;
        // std::cout << "check" << std::endl;
        for (; checkTime--;)
        {
            float mid = (l + r) / 2.0f;
            // std::cout << "l: " << l << " r: " << r << " mid: " << mid << std::endl;
            glm::vec3 pos_pre = pos;
            glm::vec3 dir_pre = dir;
            if (type == ARROW_NORMAL)
            {
                glm::vec3 g = glm::vec3(0.0f, -GRAVITY, 0.0f);
                glm::vec3 delta = dir * speed * mid + g * mid * mid / 2.0f;
                delta = delta * float(pow((1 - WIND_RESISTANCE) / weight, mid));
                pos += delta;
                dir = glm::normalize(delta);
            }
            else if (type == ARROW_LASER)
            {
                glm::vec3 g = glm::vec3(0.0f, 0.0f, 0.0f);
                glm::vec3 delta = dir * speed * mid + g * mid * mid / 2.0f;
                // delta = delta * float(pow((1 - WIND_RESISTANCE) / weight, mid));
                pos += delta;
                dir = glm::normalize(delta);
            }
            updateModel();
            if (arrow.intersectWith(control->ground.getModel()))
            {
                r = mid;
            }
            else
            {
                l = mid;
            }
            pos = pos_pre;
            dir = dir_pre;
        }
        if (type == ARROW_NORMAL)
        {
            glm::vec3 g = glm::vec3(0.0f, -GRAVITY, 0.0f);
            glm::vec3 delta = dir * speed * r + g * r * r / 2.0f;
            delta = delta * float(pow((1 - WIND_RESISTANCE) / weight, r));
            pos += delta;
            dir = glm::normalize(delta);
        }
        else if (type == ARROW_LASER)
        {
            glm::vec3 g = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 delta = dir * speed * r + g * r * r / 2.0f;
            // delta = delta * float(pow((1 - WIND_RESISTANCE) / weight, r));
            pos += delta;
            dir = glm::normalize(delta);
        }
        updateModel();
        if (arrow.intersectWith(control->ground.getModel()))
        {
            state = ARROW_HIT_WALL;
            std::cout << "pos_hit_wall: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
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

bool Arrow::fire()
{
    if (state != ARROW_HOLD)
        return false;
    this->state = ARROW_FLY;
    pressTime -= 0.2f;
    if (pressTime < 0.0f)
        pressTime = 0.0f;
    float strength = strengthMin + (strengthMax - strengthMin) * pressTime / strengthTime;
    if (strength > strengthMax)
        strength = strengthMax;
    if (type == ARROW_NORMAL)
        this->speed *= strength;
    std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    return true;
}

void Arrow::updateModel()
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    // 旋转到dir方向
    glm::vec3 axis;
    float angle;
    axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(dir.x, 0.0f, dir.z));
    angle = glm::acos(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), glm::normalize(glm::vec3(dir.x, 0.0f, dir.z))));
    if (axis.y < 0.0f)
    {
        angle = -angle;
    }
    axis = glm::vec3(0.0f, 1.0f, 0.0f);
    model = glm::rotate(model, angle, axis);
    axis = glm::vec3(1.0f, 0.0f, 0.0f);
    angle = glm::acos(dir.y);
    model = glm::rotate(model, angle, axis);
    // model = glm::rotate(model, glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    switch (type)
    {
    case ARROW_NORMAL:
        arrow_normal.setModel_noscale(model);
        break;
    case ARROW_LASER:
        arrow_laser.setModel_noscale(model);
        break;
    case ARROW_GROUND_SPIKE:
        arrow_ground_spike.setModel_noscale(model);
        break;
    }
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

ArrowManager *ArrowManager::getInstance()
{
    static ArrowManager instance;
    return &instance;
}

void ArrowManager::init(const char *objfile)
{
    obj = Scene::LoadObj(objfile);
    arrow_normal = Object(OBJECT_NONE, nullptr, nullptr, glm::mat4(1.0),
                          nullptr, false);
    for (auto &mesh : obj->getMesh())
    {
        Object *node = new Object(OBJECT_MESH, &mesh, player_shader, glm::mat4(1.0f), &arrow_normal);
    }
    arrow_laser = Object(OBJECT_NONE, nullptr, nullptr, glm::mat4(1.0),
                         nullptr, false);
    for (auto &mesh : obj->getMesh())
    {
        Object *node = new Object(OBJECT_MESH, &mesh, player_shader, glm::mat4(1.0f), &arrow_laser);
    }
    arrow_ground_spike = Object(OBJECT_NONE, nullptr, nullptr, glm::mat4(1.0),
                                nullptr, false);
    for (auto &mesh : obj->getMesh())
    {
        Object *node = new Object(OBJECT_MESH, &mesh, player_shader, glm::mat4(1.0f), &arrow_ground_spike);
    }
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
    if (control->leftPress)
    {
        Arrow &arrow = arrows[arrowMap[PLAYER_ID]];
        if (arrow.state == ARROW_HOLD)
        {
            arrow.pressTime += dt;
            if (arrow.pressTime > arrow.strengthTime)
                arrow.pressTime = arrow.strengthTime;
        }
    }
}

void ArrowManager::bindArrow(int playerId, ArrowType type, float speed, float scale, float weight, float loadTime)
{
    Arrow arrow = Arrow(arrowMgr->arrow_normal, arrowMgr->arrow_laser, arrowMgr->arrow_ground_spike);
    arrow.type = type;
    arrow.scale = scale;
    arrow.speed = speed;
    arrow.weight = weight;
    arrow.loadTime = loadTime;
    arrow.state = ARROW_NONE;
    arrow.attackerId = playerId;
    arrows[++arrowCnt] = arrow;
    if (arrowSetting.count(playerId))
        arrows.erase(arrowSetting[playerId]);
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

bool ArrowManager::fire(int playerId)
{
    if (arrowMap.find(playerId) == arrowMap.end())
        return 0;
    bool t = arrows[arrowMap[playerId]].fire();
    // std::cout << "fire time: " << pressTime << std::endl;
    // std::cout << "playerId: " << playerId << std::endl;
    if (t)
        load(playerId);
    return t;
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
