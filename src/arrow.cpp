#include "arrow.hpp"
#include "control.h"
#include "defs.h"
#include <ray.h>

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
        // std::cout << "pos_update: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
        // std::cout << "liveTime: " << liveTime << std::endl;
        if (pos.y <= FLOOR_Y)
        {
            state = ARROW_ON_FLOOR;
        }

        int checkTime = CHECK_TIME;
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
                glm::vec3 delta = velocity * mid + g * mid * mid / 2.0f;
                delta = delta * float(pow((1 - WIND_RESISTANCE) / weight, mid));
                pos += delta;
                dir = glm::normalize(delta);
            }
            else if (type == ARROW_LASER)
            {
                glm::vec3 g = glm::vec3(0.0f, 0.0f, 0.0f);
                glm::vec3 delta = velocity * mid + g * mid * mid / 2.0f;
                // delta = delta * float(pow((1 - WIND_RESISTANCE) / weight, mid));
                pos += delta;
                dir = glm::normalize(delta);
            }
            updateModel_obb();
            const Object *interObj = arrow.intersectWith(control->ground.getModel());
            if (interObj)
            {
                if (isReflect)
                {
                    Ray ray = Ray(pos - dir * 0.1f, dir);
                    IntersectPoint intersectPoint = ray.intersectWith(*interObj);
                    if (intersectPoint.inter)
                    {
                        r = mid;
                    }
                    else
                    {
                        l = mid;
                    }
                }
                else
                {
                    r = mid;
                }
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
            glm::vec3 g = glm::vec3(0.0f, -GRAVITY / 2.0f, 0.0f);
            glm::vec3 delta = velocity * r + g * r * r / 2.0f;
            delta = delta * float(pow((1 - WIND_RESISTANCE) / weight, r));
            pos += delta;
            dir = glm::normalize(delta);
            velocity = velocity + g * r;
            velocity = velocity * float(pow((1 - WIND_RESISTANCE) / weight, r));
            dir = glm::normalize(velocity);
        }
        else if (type == ARROW_LASER)
        {
            glm::vec3 g = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 delta = velocity * r + g * r * r / 2.0f;
            // delta = delta * float(pow((1 - WIND_RESISTANCE) / weight, r));
            pos += delta;
            dir = glm::normalize(delta);
            velocity = velocity + g * r;
            velocity = velocity * float(pow((1 - WIND_RESISTANCE) / weight, r));
            dir = glm::normalize(velocity);
        }
        updateModel_obb();
        const Object *interObj = arrow.intersectWith(control->ground.getModel());
        if (interObj)
        {
            if (!isReflect)
            {
                state = ARROW_HIT_WALL;
                //                std::cout << "pos_hit_wall: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
            }
            else
            {
                Ray ray = Ray(pos - dir * 0.1f, dir);
                IntersectPoint intersectPoint = ray.intersectWith(*interObj);
                if (intersectPoint.inter)
                {
                    float speed = glm::length(velocity);
                    dir = glm::normalize(glm::reflect(dir, intersectPoint.n));
                    if (type == ARROW_LASER)
                        velocity = dir * speed;
                    else if (type == ARROW_NORMAL)
                    {
                        velocity = dir * speed * ELASTICITY;
                        if (glm::length(velocity) < EPS)
                        {
                            dir = glm::normalize(glm::reflect(dir, intersectPoint.n));
                            state = ARROW_HIT_WALL;
                        }
                    }
                }
            }
        }
        if (dt - r > EPS)
        {
            update(dt - r);
            dt = r;
        }
        for (auto &player : control->players)
        {
            if (player.id == attackerId)
                continue;
            if (arrow.intersectWith(player.getBody()) || arrow.intersectWith(player.getHead()))
            {
                state = ARROW_HIT_PLAYER;
                hitPlayerId = player.id;
            }
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
    if (state == ARROW_HIT_WALL | state == ARROW_ON_PLAYER | state == ARROW_ON_FLOOR)
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
    if (state == ARROW_HOLD | state == ARROW_ON_PLAYER | state == ARROW_LOADING)
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
    velocity = dir * speed;
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

void Arrow::updateModel_obb()
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
        arrow_normal.setLModelObb(model);
        break;
    case ARROW_LASER:
        arrow_laser.setLModelObb(model);
        break;
    case ARROW_GROUND_SPIKE:
        arrow_ground_spike.setLModelObb(model);
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

void ArrowManager::updateModel()
{
    for (auto &[_, arrow] : arrows)
    {
        arrow.updateModel();
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
        if (it->second.state == ARROW_HIT_PLAYER)
        {
            it->second.state = ARROW_ON_PLAYER;
            glm::vec3 hitPos = it->second.pos;
            glm::vec3 hitDir = it->second.dir;
            glm::vec3 posOffset = hitPos - control->players[it->second.hitPlayerId].position;
            glm::vec3 dirOffset = hitDir - control->players[it->second.hitPlayerId].right;
            arrowHit[it->second.hitPlayerId].push_back({it->second.id, {posOffset, dirOffset}});
            std::cout << "id: " << it->second.id << std::endl;
            std::cout << "hitPlayerId: " << it->second.hitPlayerId << std::endl;
            std::cout << "posOffset: " << posOffset.x << " " << posOffset.y << " " << posOffset.z << std::endl;
            std::cout << "dirOffset: " << dirOffset.x << " " << dirOffset.y << " " << dirOffset.z << std::endl;
        }
        if (it->second.state == ARROW_DISAPPEAR)
        {
            if (it->second.hitPlayerId != -1)
            {
                for (auto &arrow : arrowHit[it->second.hitPlayerId])
                {
                    if (arrow.first == it->second.id)
                    {
                        arrowHit[it->second.hitPlayerId].erase(arrowHit[it->second.hitPlayerId].begin() + (&arrow - &arrowHit[it->second.hitPlayerId][0]));
                        break;
                    }
                }
            }
            it = arrows.erase(it);
        }
        else
        {
            it++;
        }
    }
    if (control->leftMousePress)
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
    arrow.id = ++arrowCnt;
    arrows[arrowCnt] = arrow;
    if (arrowSetting.count(playerId))
        arrows.erase(arrowSetting[playerId]);
    arrowSetting[playerId] = arrowCnt;
    load(playerId);
}

void ArrowManager::load(int playerId)
{
    if (arrowSetting.find(playerId) == arrowSetting.end())
        assert(false);
    if (arrowMap.count(playerId))
        arrows.erase(arrowMap[playerId]);
    arrows[++arrowCnt] = arrows[arrowSetting[playerId]];
    arrows[arrowCnt].id = arrowCnt;
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
    {
        arrowMap.erase(playerId);
        load(playerId);
    }
    return t;
}

void ArrowManager::updateArrow(int playerId, glm::vec3 pos, glm::vec3 dir)
{
    if (arrowMap.count(playerId))
        arrows[arrowMap[playerId]].update(pos, dir);
    if (arrowHit.count(playerId))
    {
        glm::vec3 playerPos = control->players[playerId].position;
        glm::vec3 playerDir = control->players[playerId].right;
        for (auto &[arrowId, offset] : arrowHit[playerId])
        {
            arrows[arrowId].update(playerPos + offset.first, playerDir + offset.second);
        }
    }
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
