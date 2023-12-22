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

class Player
{
public:
    Player();
    ~Player();

    void init(const char *objfile, glm::vec3 position);

    void processKeyboard();
    void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void jump();
    enum PlayerState
    {
        PLAYER_STILL = 0,
        PLAYER_RUN,
    };

    void draw();
    void update(float dt);

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
    bool poschanged;

    void fire();
    // void press();
    Object &getBody()
    {
        return body;
    }
	Object& getHead()
	{
		return head;
	}
	void getCandy(CandyType type);
    int id;

private:
    bool checkBlocked(enum intersectType type);
    void updatey(float dt);
    bool navigate(float speedfactor, float anglefactor, float dt);
    void updateModel();
    void updatePlayerVectors();

    Scene *obj; // 用于析构

    Object head;
    Object body;
    Object larm, rarm;
    Object lleg, rleg;

    float yaw;
    float pitch;
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
};

#endif
