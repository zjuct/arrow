#ifndef _PLAYER_H
#define _PLAYER_H

#include "objLoader.hpp"
#include "object.hpp"
#include "camera.hpp"

class Player {
public:
	Player();
	~Player();

	void init(const char* objfile);

	void processKeyboard(Movement direction, float deltaTime);
	void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

	enum PlayerState {
		PLAYER_STILL = 0,
		PLAYER_RUN,
	};

	void draw();
	void update(float dt);

	void setState(PlayerState state) {
		this->state = state;
	}

	PlayerState state = PLAYER_STILL;

	glm::vec3 getPosition() {
		return position;
	}

	glm::vec3 getFront() {
		return front;
	}
	
private:
	void updateModel();
	void updatePlayerVectors();

	Scene* obj;			// 用于析构

	Object head;
	Object body;
	Object larm, rarm;
	Object lleg, rleg;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	float yaw;
	float pitch;
	float speed;
  float sensitivity;

	float omega = 1.0f;		// 角速度
	float theta = 0.0f;		// 手臂和腿转动
};

#endif
