#ifndef _PLAYER_H
#define _PLAYER_H

#include "objLoader.hpp"
#include "object.hpp"

class Player {
public:
	Player();
	~Player();

	void init(const char* objfile);

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
	
private:
	void updateModel();

	Scene* obj;			// 用于析构

	Object head;
	Object body;
	Object larm, rarm;
	Object lleg, rleg;

	float omega = 0.5f;		// 角速度
	float theta = 0.0f;		// 手臂和腿转动
};

#endif
