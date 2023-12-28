#include "arrow.hpp"
#include "control.h"
#include "defs.h"
#include <ray.h>
#include <sync.hpp>
#include <winsock2.h>

static Control *control = Control::getInstance();
static ArrowManager *arrowMgr = ArrowManager::getInstance();

extern SOCKET sock;

Arrow::Arrow(Object arrow_normal, Object arrow_laser, Object arrow_ground_spike)
{
    this->arrow_normal = arrow_normal;
    this->arrow_laser = arrow_laser;
    this->arrow_ground_spike = arrow_ground_spike;
}

void Arrow::draw(Shader *shader)
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
        arrow_normal.draw(shader);
        break;
    case ARROW_LASER:
        arrow_laser.draw(shader);
        break;
    case ARROW_GROUND_SPIKE:
        arrow_ground_spike.draw(shader);
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

#if GRID_ENABLE
            const Object *interObj = control->grid.intersectWith(arrow);
#else
            const Object *interObj = arrow.intersectWith(control->ground.getModel());
#endif
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
#if GRID_ENABLE
        const Object *interObj = control->grid.intersectWith(arrow);
#else
        const Object *interObj = arrow.intersectWith(control->ground.getModel());
#endif
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
        updateModel_obb();
        for (auto &[_, player] : control->players)
        {
            if (player.id == attackerId)
                continue;
            if (arrow.intersectWith(player.getBody()) || arrow.intersectWith(player.getHead()))
            {
                state = ARROW_HIT_PLAYER;
                hitPlayerId = player.id;
            }
        }
        if (dt - r > EPS)
        {
            update(dt - r);
            dt = r;
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

void ArrowManager::draw(Shader *shader)
{
    for (auto &[_, arrow] : arrows)
    {
        arrow.draw(shader);
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
            glm::vec3 right = control->players[it->second.hitPlayerId].right;
            float rightAngle = glm::atan(right.z, right.x);
            arrowHit[it->second.hitPlayerId].push_back({it->second.id, posOffset, hitDir, rightAngle});
            std::cout << "id: " << it->second.id << std::endl;
            std::cout << "hitPlayerId: " << it->second.hitPlayerId << std::endl;
            std::cout << "posOffset: " << posOffset.x << " " << posOffset.y << " " << posOffset.z << std::endl;
            control->players[it->second.hitPlayerId].getHit(it->second);
        }
        if (it->second.state == ARROW_DISAPPEAR)
        {
            if (it->second.hitPlayerId != -1)
            {
                for (auto &arrow : arrowHit[it->second.hitPlayerId])
                {
                    if (std::get<0>(arrow) == it->second.id)
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
    FuncSyncPackage funcSyncPackage = FuncSyncPackage(FUNC_ARROW_BIND, &playerId, &type, &speed, &scale, &weight, &loadTime);
    if (playerId == PLAYER_ID)
        funcSyncPackage.send(sock);
}

void ArrowManager::bindArrow(FuncSyncPackage &funcSyncPackage)
{
    int playerId;
    ArrowType type;
    float speed, scale, weight, loadTime;
    funcSyncPackage.get(&playerId, &type, &speed, &scale, &weight, &loadTime);
    bindArrow(playerId, type, speed, scale, weight, loadTime);
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
    // FuncSyncPackage funcSyncPackage = FuncSyncPackage(FUNC_ARROW_LOAD, &playerId);
    // if (playerId == PLAYER_ID)
    //     funcSyncPackage.send(sock);
}

void ArrowManager::load(FuncSyncPackage &funcSyncPackage)
{
    // int playerId;
    // funcSyncPackage.get(&playerId);
    // load(playerId);
}

bool ArrowManager::fire(int playerId)
{
    if (arrowMap.find(playerId) == arrowMap.end())
        return 0;
    Arrow &arrow = arrows[arrowMap[playerId]];
    FuncSyncPackage funcSyncPackage = FuncSyncPackage(FUNC_ARROW_FIRE, &playerId, &arrow.pos, &arrow.dir, &arrow.pressTime);
    if (playerId == PLAYER_ID)
        funcSyncPackage.send(sock);
    std::cout << "pos: " << arrow.pos.x << " " << arrow.pos.y << " " << arrow.pos.z << std::endl;
    std::cout << "dir: " << arrow.dir.x << " " << arrow.dir.y << " " << arrow.dir.z << std::endl;
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

void ArrowManager::fire(FuncSyncPackage &funcSyncPackage)
{
    int playerId;
    glm::vec3 pos, dir;
    float pressTime;
    funcSyncPackage.get(&playerId, &pos, &dir, &pressTime);
    std::cout << "pos: " << pos.x << " " << pos.y << " " << pos.z << std::endl;
    std::cout << "dir: " << dir.x << " " << dir.y << " " << dir.z << std::endl;
    std::cout << "pressTime: " << pressTime << std::endl;
    updateArrow(playerId, pos, dir);
    arrows[arrowMap[playerId]].pressTime = pressTime;
    fire(playerId);
}

void ArrowManager::updateArrow(int playerId, glm::vec3 pos, glm::vec3 dir)
{
    if (arrowMap.count(playerId))
        arrows[arrowMap[playerId]].update(pos, dir);
    if (arrowHit.count(playerId))
    {
        glm::vec3 playerPos = control->players[playerId].position;
        glm::vec3 playerDir = control->players[playerId].right;
        float playerRightAngle = glm::atan(playerDir.z, playerDir.x);
        for (auto &[arrowId, offset, dir, rightAngle] : arrowHit[playerId])
        {
            float angle = playerRightAngle - rightAngle;
            glm::vec3 nowOffset = glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(offset, 1.0f);
            glm::vec3 nowDir = glm::rotate(glm::mat4(1.0f), -angle, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(dir, 1.0f);
            arrows[arrowId].update(playerPos + nowOffset, nowDir);
        }
    }
    // FuncSyncPackage funcSyncPackage = FuncSyncPackage(FUNC_ARROW_UPDATE, &playerId, &pos, &dir);
    // if (playerId == PLAYER_ID)
    //     funcSyncPackage.send(sock);
}

// void ArrowManager::updateArrow(FuncSyncPackage &funcSyncPackage)
// {
//     int playerId;
//     glm::vec3 pos, dir;
//     funcSyncPackage.get(&playerId, &pos, &dir);
//     updateArrow(playerId, pos, dir);
// }

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

ArrowSyncPackage::ArrowSyncPackage(Arrow *arrow)
{
    // pos, dir, speed, scale, weight, isReflect, liveTime, disappearTime, id, type
    type = Sync_Arrow;
    timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    size = sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool) + sizeof(int) * 3 + sizeof(ArrowType);
    data = new char[size];
    memset(data, 0, size);
    memcpy(data, &arrow->pos, sizeof(glm::vec3));
    memcpy(data + sizeof(glm::vec3), &arrow->dir, sizeof(glm::vec3));
    memcpy(data + sizeof(glm::vec3) * 2, &arrow->speed, sizeof(float));
    memcpy(data + sizeof(glm::vec3) * 2 + sizeof(float), &arrow->scale, sizeof(float));
    memcpy(data + sizeof(glm::vec3) * 2 + sizeof(float) * 2, &arrow->weight, sizeof(float));
    memcpy(data + sizeof(glm::vec3) * 2 + sizeof(float) * 3, &arrow->isReflect, sizeof(bool));
    memcpy(data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool), &arrow->liveTime, sizeof(int));
    memcpy(data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool) + sizeof(int), &arrow->disappearTime, sizeof(int));
    memcpy(data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool) + sizeof(int) * 2, &arrow->attackerId, sizeof(int));
    memcpy(data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool) + sizeof(int) * 3, &arrow->type, sizeof(ArrowType));
}

void ArrowSyncPackage::update(Arrow *arrow)
{
    memcpy(&arrow->pos, data, sizeof(glm::vec3));
    memcpy(&arrow->dir, data + sizeof(glm::vec3), sizeof(glm::vec3));
    memcpy(&arrow->speed, data + sizeof(glm::vec3) * 2, sizeof(float));
    memcpy(&arrow->scale, data + sizeof(glm::vec3) * 2 + sizeof(float), sizeof(float));
    memcpy(&arrow->weight, data + sizeof(glm::vec3) * 2 + sizeof(float) * 2, sizeof(float));
    memcpy(&arrow->isReflect, data + sizeof(glm::vec3) * 2 + sizeof(float) * 3, sizeof(bool));
    memcpy(&arrow->liveTime, data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool), sizeof(int));
    memcpy(&arrow->disappearTime, data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool) + sizeof(int), sizeof(int));
    memcpy(&arrow->attackerId, data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool) + sizeof(int) * 2, sizeof(int));
    memcpy(&arrow->type, data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool) + sizeof(int) * 3, sizeof(ArrowType));
}

int ArrowSyncPackage::getId()
{
    return *(int *)(data + sizeof(glm::vec3) * 2 + sizeof(float) * 3 + sizeof(bool) + sizeof(int) * 2);
}
