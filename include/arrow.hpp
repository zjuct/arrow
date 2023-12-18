#ifndef _ARROW_H
#define _ARROW_H

#include "objLoader.hpp"
#include "object.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>

enum ArrowState {
    ARROW_HOLD = 0,
    ARROW_FLY,
    ARROW_HIT_PLAYER,
    ARROW_HIT_WALL,
    ARROW_ON_FLOOR,
    ARROW_STOP,
    ARROW_DISAPPEAR,
};

enum ArrowType {
    ARROW_NORMAL = 0,
    ARROW_LASER,        // 激光
    ARROW_GROUND_SPIKE, // 地刺
};

class Arrow {

public:
    Scene* obj;

    Object arrow_normal;
    Object arrow_laser;
    Object arrow_ground_spike;


    bool isFire = false;
    bool isStop = false;
    bool isReflect = false;
    float scale = 1.0f;
    float speed = 1.0f;
    float weight = 1.0f;

    float liveTime = 0.0f;
    
    float stopTime = 0.0f;
    float disappearTime = 5.0f;

    glm::vec3 pos;
    glm::vec3 dir;
    
    ArrowState state = ARROW_HOLD;
    ArrowType type = ARROW_NORMAL;

public:
    Arrow(){}
    ~Arrow(){}

    void init(const char* objfile);
    void draw();
    void update(float dt);
    void updateModel();
    void fire(glm::vec3 pos, glm::vec3 dir, float scale, float speed);
    // void stop();
    // void disappear();

    
};



class ArrowManager {
public:
    static ArrowManager* getInstance();

    ArrowManager(){}
    ~ArrowManager(){}

    void init(const char* objfile);
    // void draw();
    // void update(float dt);
    // void fire(glm::vec3 pos, glm::vec3 dir, float scale, float speed);
    // void stop();
    // void disappear();

    std::vector<Arrow> arrows;
};


#endif