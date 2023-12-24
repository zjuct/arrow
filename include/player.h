#ifndef _PLAYER_H
#define _PLAYER_H

#include "objLoader.hpp"
#include "object.hpp"
#include <candy.hpp>

enum Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    DOWN,
    UP,
    STILL,
};

class Arrow;

class Player
{
public:
    Player();
    ~Player();

    void init(const char *objfile, glm::vec3 position);

    void processKeyboard();
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void jump();
    void rebirth();
    enum PlayerState
    {
        PLAYER_STILL = 0,
        PLAYER_RUN,
        PLAYER_DEAD,
    };

    void draw();
    void updateModel();
    void update(float dt);

    void updateModel_obb();

    void setState(PlayerState state)
    {
        this->state = state;
    }

    PlayerState state = PLAYER_STILL;

    glm::vec3 getPosition()
    {
        return position;
    }

    glm::vec3 getFront()
    {
        return front;
    }

    glm::vec3 getRight()
    {
        return right;
    }

    glm::vec3 getUp()
    {
        return up;
    }

    glm::vec3 getWeaponPos()
    {
        return position + up * 0.25f + right * 0.3f;
    }

    void setLastYaw()
    {
        lastyaw = yaw;
    }

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

    void fire();
    // void press();
    Object &getBody()
    {
        return body;
    }
    Object &getHead()
    {
        return head;
    }
    void getCandy(CandyType type);
    int id;
    int getHp()
    {
        return hp + 0.5f;
    }
    int getMaxHp()
    {
        return maxHp;
    }
    int getLevel()
    {
        return level;
    }
    void getHit(const Arrow &arrow);
    float yaw;
    float pitch;

    float hp = 100.0f;
    int exp = 0;
    int level = 1;

private:
    bool checkBlocked(enum intersectType type);
    void updatey(float dt);
    bool navigate(float speedfactor, float anglefactor, float dt);
    void updatePlayerVectors();

    Scene *obj; // 用于析构

    Object head;
    Object body;
    Object larm, rarm;
    Object lleg, rleg;

    float speed;
    float sensitivity;

    int jumpTime = 2;
    int maxJumpTime = 2;
    bool floating = false;
    float jumpHeight = 3.0f;
    float jumpSpeed = 0.0f;
    float fireTime = 0.0f;

    float lastyaw;

    float omega = 1.0f; // 角速度
    float theta = 0.0f; // 手臂和腿转动

    glm::vec3 moveDir;
    glm::vec3 inputDir;

    float frontSpeed = 0.0f;
    float rightSpeed = 0.0f;

    float maxHp = 100.0f;
    float hpRecover = 0.1f;
    float hpRecoverTime = 0.0f;
    int maxLevel = 20;
    int expToLevelUp[20] = {0, 110, 120, 130, 140, 150, 160, 170, 180, 190,
                            200, 210, 220, 230, 240, 250, 260, 270, 280, 290};
    int candyExp = 10;
};

#endif
