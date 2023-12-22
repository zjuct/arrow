#ifndef _ARROW_H
#define _ARROW_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "objLoader.hpp"
#include "object.hpp"
#include <map>
#include <vector>

enum ArrowState
{
    ARROW_NONE = 0,
    ARROW_LOADING,
    ARROW_HOLD,
    ARROW_FLY,
    ARROW_HIT_PLAYER,
    ARROW_HIT_WALL,
    ARROW_ON_PLAYER,
    ARROW_ON_FLOOR,
    ARROW_SPIKE_LIVE,
    ARROW_STOP,
    ARROW_DISAPPEAR,
};

enum ArrowType
{
    ARROW_NORMAL = 0,
    ARROW_LASER,        // 激光
    ARROW_GROUND_SPIKE, // 地刺
};

class Arrow
{

public:
    Object arrow_normal;
    Object arrow_laser;
    Object arrow_ground_spike;

    bool isFire = false;
    bool isStop = false;
    bool isReflect = false;
    float scale = 1.0f;
    float speed = 1.0f;
    float weight = 1.0f;
    float loadTime = 0.1f;
    float pressTime = 0.0f;

    float liveTime = 20.0f;
    // float stopTime = 0.0f;
    float disappearTime = 10.0f;

    float strengthMax = 3.0f;
    float strengthMin = 1.0f;
    float strengthTime = 2.0f;
    float strength = 1.0f;

    float damage = 1.0f;

    glm::vec3 pos;
    glm::vec3 dir;
    glm::vec3 velocity;

    ArrowState state = ARROW_HOLD;
    ArrowType type = ARROW_NORMAL;

    int attackerId = -1;
    int id;
    int hitPlayerId = -1;

public:
    Arrow() {}
    Arrow(Object arrow_normal, Object arrow_laser, Object arrow_ground_spike);
    ~Arrow() {}

    // Arrow(const Arrow &arrow);

    // void init(const char* objfile);
    void draw();
    void update(float dt);
    void update(glm::vec3 pos, glm::vec3 dir);
    void updateModel();
    void updateModel_obb();
    bool fire();
    // void stop();
    // void disappear();
};

class ArrowManager
{
public:
    static ArrowManager *getInstance();

    Scene *obj;

    Object arrow_normal;
    Object arrow_laser;
    Object arrow_ground_spike;
    int arrowCnt = 0;
    std::map<int, Arrow> arrows;
    std::map<int, int> arrowMap;
    std::map<int, int> arrowSetting;
    std::map<int, std::vector<std::pair<int, std::pair<glm::vec3, glm::vec3>>>> arrowHit;

    ArrowManager() {}
    ~ArrowManager() {}

    void init(const char *objfile);
    void draw();
    void updateModel();
    void update(float dt);
    void updateModel_obb();
    // void stop();
    // void disappear();
    void bindArrow(int playerId, ArrowType type = ARROW_NORMAL, float speed = 15.0f, float scale = 1.0f, float weight = 1.0f, float loadTime = 0.1f);
    void updateArrow(int playerId, glm::vec3 pos, glm::vec3 dir);
    void deleteArrow(int playerId);
    bool fire(int playerId);
    void load(int playerId);

    Arrow &getArrowSetting(int playerId)
    {
        return arrows[arrowSetting[playerId]];
    }
};

#endif