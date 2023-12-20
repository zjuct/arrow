#ifndef _CANDY_H
#define _CANDY_H

#include "objLoader.hpp"
#include "object.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include <map>
#include <set>
#include <list>

enum CandyType
{
    CANDY_SPEED_UP = 0,
    CANDY_JUMP_HEIGHT_UP,
    CANDY_JUMP_TIME_UP,
    CANDY_ARROW_SPEED_UP,
    CANDY_ARROW_LOAD_TIME_DOWN,
    CANDY_ARROW_STRENGTH_TIME_DOWN,
    CANDY_ARROW_SCALE_UP,
    CANDY_ARROW_FIRE,
    CANDY_ARROW_LIVE_TIME_UP,
    CANDY_ARROW_REFLECT,
    CANDY_ARROW_LASER,
    CANDY_ARROW_GROUND_SPIKE,
    CANDY_TYPE_NUM,
    CANDY_NONE,
    CANDY_DISAPPEAR,
};

class Candy
{
public:
    Object candy;
    constexpr static float rotateSpeed = 0.5f;
    float liveTime = 10.0f;
    glm::vec3 pos;
    float rotateDir;
    float scale = 1.0f;

    CandyType type = CANDY_NONE;
    Candy()
    {
        pos = glm::vec3(0.0f, 0.0f, 0.0f);
        rotateDir = 0.0f;
        scale = 1.0f;
        type = CANDY_NONE;
    }
    Candy(Object &candy, glm::vec3 pos, CandyType type, float rotateDir = 0.0f, float scale = 1.0f)
    : candy(candy)
    {
        this->pos = pos;
        this->type = type;
        this->rotateDir = 1.0f * (rand() % 360);
        this->scale = scale;
    }

    void draw();
    void update(float dt);
    void updateModel();
    void hit();
};

class CandyManager
{
public:
    static CandyManager *getInstance();

    Scene *obj;
    Object model;
    std::list<Candy> candies;

    float generateTime = 1.0f;

    void update(float dt);
    void init(const char *objfile);
    void draw();
    void generateCandy(glm::vec3 pos, CandyType type);
    void generateCandy();
};

#endif