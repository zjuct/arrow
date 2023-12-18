#include "arrow.hpp"
#include "defs.h"
#include "control.h"

static Control *control = Control::getInstance();
static ArrowManager *arrowMgr = ArrowManager::getInstance();

void Arrow::init(const char *objfile)
{
    obj = Scene::LoadObj(objfile);
    std::vector<Mesh> &meshes = obj->getMesh();
    // .obj中定义的顺序必须为arrow_normal, arrow_laser, arrow_ground_spike
    arrow_normal = Object(OBJECT_MESH, &meshes[0], player_shader);
    arrow_laser = Object(OBJECT_MESH, &meshes[1], player_shader);
    arrow_ground_spike = Object(OBJECT_MESH, &meshes[2], player_shader);
}

void Arrow::draw()
{
    updateModel();
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
    if (state == ARROW_HOLD)
        return;
    if (state == ARROW_FLY)
    {
        glm::vec3 g = glm::vec3(0.0f, -GRAVITY, 0.0f);
        glm::vec3 delta = dir * speed + g * dt * weight / WIND_RESISTANCE;
        pos += delta;
        dir = glm::normalize(delta);
    }
}

void Arrow::fire(glm::vec3 pos, glm::vec3 dir, float scale, float speed)
{
    this->pos = pos;
    this->dir = dir;
    this->speed = speed;
    this->state = ARROW_FLY;
}

void Arrow::updateModel()
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, glm::vec3(scale, scale, scale));
}

static ArrowManager *instance = nullptr;
ArrowManager *ArrowManager::getInstance()
{
    static ArrowManager instance;
    return &instance;
}

void ArrowManager::init(const char *objfile)
{
    
}
