#include "player.h"
#include "shader.h"
#include <defs.h>
#include <control.h>

#include <vector>

#include <iostream>

static Control *control = Control::getInstance();

Player::Player() : position(glm::vec3(0.0f)), speed(2.5f), sensitivity(0.1f), yaw(-90.0f), pitch(0.0f)
{
}

Player::~Player()
{
}

void Player::init(const char *objfile, glm::vec3 position)
{
	this->position = position;
	updatePlayerVectors();

	obj = Scene::LoadObj(objfile);
	std::vector<Mesh> &meshes = obj->getMesh();
	// .obj中定义的顺序必须为body, head, larm, lleg, rarm, rleg
	body = Object(OBJECT_MESH, &meshes[0], player_shader);
	head = Object(OBJECT_MESH, &meshes[1], player_shader);
	larm = Object(OBJECT_MESH, &meshes[2], player_shader);
	lleg = Object(OBJECT_MESH, &meshes[3], player_shader);
	rarm = Object(OBJECT_MESH, &meshes[4], player_shader);
	rleg = Object(OBJECT_MESH, &meshes[5], player_shader);
}

void Player::processKeyboard(Movement direction, float deltaTime)
{
	float velocity = speed * deltaTime;
	if (direction == FORWARD)
		position += glm::vec3(front.x, 0.0f, front.z) * velocity;
	if (direction == BACKWARD)
		position -= glm::vec3(front.x, 0.0f, front.z) * velocity;
	if (direction == LEFT)
		position -= right * velocity;
	if (direction == RIGHT)
		position += right * velocity;
	// if (direction == DOWN)
	// 		position -= up * velocity;
}
void Player::jump()
{
	std::cout << "jump" << std::endl;
	if (jumpTime <= 0)
		return;
	--jumpTime;
	jumpSpeed = 1.0f * jumpHeight;
	std::cout << "time: " << jumpTime << std::endl;
}
void Player::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (pitch > 30.0f)
			pitch = 30.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;
	}

	updatePlayerVectors();
}

// 根据 pitch 和 yaw 修改 front right up 等
void Player::updatePlayerVectors()
{
	// calculate the new Front vector
	glm::vec3 f;

	// 这里调用的是 cmath 库中的三角函数，输入为弧度制
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = glm::sin(glm::radians(pitch));
	f.z = glm::sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	this->front = glm::normalize(f);
	// also re-calculate the Right and Up vector
	this->right = glm::normalize(glm::cross(front, control->camera.WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	this->up = glm::normalize(glm::cross(right, front));
}

void Player::draw()
{
	updateModel();
	head.draw();
	body.draw();
	larm.draw();
	rarm.draw();
	lleg.draw();
	rleg.draw();
}

void Player::update(float dt)
{
	switch (this->state)
	{
	case PLAYER_STILL:
		theta = 0.0f;
		break;
	case PLAYER_RUN:
		// std::cout << "update" << std::endl;
		theta += omega * dt * 3.0f;
		if (theta > M_PIf / 6.0f)
		{
			theta = M_PIf / 6.0f;
			omega = -omega;
		}
		else if (theta < -M_PIf / 6.0f)
		{
			theta = -M_PIf / 6.0f;
			omega = -omega;
		}
		break;
	}

	jumpSpeed -= GRAVITY * dt * 100.0f;
	position.y += jumpSpeed * dt;
	// std::cout<<"jumpSpeed: "<<jumpSpeed<<std::endl;
	if (position.y <= FLOOR_Y)
	{
		jumpSpeed = 0.0f;
		position.y = FLOOR_Y;
		jumpTime = 2;
	}
}

void Player::updateModel()
{

	glm::mat4 basemodel(1.0f);
	basemodel = glm::translate(basemodel, position);
	// 这里的 rotate 输入为弧度制
	basemodel = glm::rotate(basemodel, (float)glm::radians(yaw + 90), glm::vec3(0.0f, -1.0f, 0.0f));

	// head: 世界坐标平移 * 局部坐标平移 * 整体模型转动 * 头俯仰角转动
	glm::mat4 model = basemodel;
	// model = glm::rotate(model, , right);
	// model = glm::rotate(model, control->camera.Pitch, glm::vec3(0.0f, 0.0f, 1.0f));
	head.setModel(model);
	head.setModel_noscale(model);

	// body: 世界坐标平移 * 局部坐标平移 * 整体模型转动
	model = basemodel;
	// model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	body.setModel(model);
	body.setModel_noscale(model);

	// larm, rarm, lleg, rleg: 世界坐标平移 * 局部坐标平移 * 整体模型转动 * 手臂/腿部转动
	model = basemodel;
	// model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, theta, glm::vec3(1.0f, 0.0f, 0.0f));
	larm.setModel(model);
	larm.setModel_noscale(model);

	model = basemodel;
	// model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -theta, glm::vec3(1.0f, 0.0f, 0.0f));
	rarm.setModel(model);
	rarm.setModel_noscale(model);

	model = basemodel;
	// model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -theta, glm::vec3(1.0f, 0.0f, 0.0f));
	lleg.setModel(model);
	rleg.setModel_noscale(model);

	model = basemodel;
	// model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, theta, glm::vec3(1.0f, 0.0f, 0.0f));
	rleg.setModel(model);
	rleg.setModel_noscale(model);
}