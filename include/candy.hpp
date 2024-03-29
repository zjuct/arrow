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
    CANDY_NONE = -1,
    CANDY_NORMAL = 0,
    // CANDY_SPEED_UP,
    // CANDY_JUMP_HEIGHT_UP,
    // CANDY_JUMP_TIME_UP,
    // CANDY_ARROW_SPEED_UP,
    // CANDY_ARROW_LOAD_TIME_DOWN,
    // CANDY_ARROW_STRENGTH_TIME_DOWN,
    // CANDY_ARROW_SCALE_UP,
    // CANDY_ARROW_FIRE,
    // CANDY_ARROW_LIVE_TIME_UP,
    // CANDY_ARROW_REFLECT,
    // CANDY_ARROW_LASER,
    // CANDY_ARROW_GROUND_SPIKE,
    CANDY_TYPE_NUM,
    CANDY_DISAPPEARING,
    CANDY_DISAPPEAR,
};

class FuncSyncPackage;

class Candy
{
public:
    Object candy;
    constexpr static float rotateSpeed = 0.5f;
    float liveTime = 100.0f;
    glm::vec3 pos;
    float rotateDir;
    float scale = 1.0f;
    int id;
    static int idCnt;

    CandyType type = CANDY_NONE;
    Candy()
    {
        pos = glm::vec3(0.0f, 0.0f, 0.0f);
        rotateDir = 0.0f;
        scale = 1.0f;
        type = CANDY_NONE;
        id = idCnt++;
    }
    Candy(Object &candy, glm::vec3 pos, CandyType type, float rotateDir = 0.0f, float scale = 1.0f)
    : candy(candy)
    {
        this->pos = pos;
        this->type = type;
        this->rotateDir = 1.0f * (rand() % 360);
        this->scale = scale;
        id = idCnt++;
    }

    void draw(Shader* shader);
    void update(float dt);
    void updateModel();
    void hit();
};

class Player;

class CandyManager
{
public:
    static CandyManager *getInstance();

    Scene *obj;
    Object model;
    std::list<Candy> candies;

    const float generateTimeMax = 1.0f;
    float generateTime = generateTimeMax;

    void update(float dt);
    void init(const char *objfile);
    void draw(Shader* shader);
    void updateModel();
    void generateCandy(glm::vec3 pos, CandyType type);
    void generateCandy();
    void generateCandy(FuncSyncPackage &package);

    void eat(Player &player);
    void eaten(FuncSyncPackage &package);
    FuncSyncPackage touch(FuncSyncPackage &package);
};

#endif