#include "player.h"
#include "shader.h"
#include <defs.h>
#include <control.h>

static Control* control = Control::getInstance();

Player::Player() {

}

Player::~Player() {

}

void Player::init(const char* objfile) {
	obj = Scene::LoadObj(objfile);
	std::vector<Mesh>& meshes = obj->getMesh();
	// .obj�ж����˳�����Ϊhead, body, larm, rarm, lleg, rleg
	head = Object(OBJECT_MESH, &meshes[0], player_shader);
	body = Object(OBJECT_MESH, &meshes[1], player_shader);
	larm = Object(OBJECT_MESH, &meshes[2], player_shader);
	rarm = Object(OBJECT_MESH, &meshes[3], player_shader);
	lleg = Object(OBJECT_MESH, &meshes[4], player_shader);
	rleg = Object(OBJECT_MESH, &meshes[5], player_shader);
}

void Player::draw() {
	updateModel();
	head.draw();
	body.draw();
	larm.draw();
	rarm.draw();
	lleg.draw();
	rleg.draw();
}

void Player::update(float dt) {
	switch (this->state) {
	case PLAYER_STILL:
		theta = 0.0f;
		break;
	case PLAYER_RUN:
		theta += omega * dt;
		if (theta > M_PIf / 6.0f) {
			theta = M_PIf / 6.0f;
			omega = -omega;
		}
		else if (theta < -M_PIf / 6.0f) {
			theta = -M_PIf / 6.0f;
			omega = -omega;
		}
		break;
	}
}

void Player::updateModel() {
	// head: ��������ƽ�� * �ֲ�����ƽ�� * ����ģ��ת�� * ͷ������ת��
	glm::mat4 model(1.0f);
	model = glm::translate(model, control->camera.Position);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));		// �ĳ�head�ľֲ�����
	model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, control->camera.Pitch, glm::vec3(0.0f, 0.0f, 1.0f));
	head.setModel(model);
	head.setModel_noscale(model);

	// body: ��������ƽ�� * �ֲ�����ƽ�� * ����ģ��ת��
	model = glm::mat4(1.0f);
	model = glm::translate(model, control->camera.Position);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));		// �ĳ�body�ľֲ�����
	model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	body.setModel(model);
	body.setModel_noscale(model);

	// larm, rarm, lleg, rleg: ��������ƽ�� * �ֲ�����ƽ�� * ����ģ��ת�� * �ֱ�/�Ȳ�ת��
	model = glm::mat4(1.0f);
	model = glm::translate(model, control->camera.Position);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));		// �ĳ�larm�ľֲ�����
	model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, theta, glm::vec3(0.0f, 0.0f, 1.0f));
	larm.setModel(model);
	larm.setModel_noscale(model);
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, control->camera.Position);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));		// �ĳ�rarm�ľֲ�����
	model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -theta, glm::vec3(0.0f, 0.0f, 1.0f));
	rarm.setModel(model);
	rarm.setModel_noscale(model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, control->camera.Position);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));		// �ĳ�lleg�ľֲ�����
	model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -theta, glm::vec3(0.0f, 0.0f, 1.0f));
	lleg.setModel(model);
	rleg.setModel_noscale(model);

	model = glm::mat4(1.0f);
	model = glm::translate(model, control->camera.Position);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));		// �ĳ�rleg�ľֲ�����
	model = glm::rotate(model, control->camera.Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, theta, glm::vec3(0.0f, 0.0f, 1.0f));
	rleg.setModel(model);
	rleg.setModel_noscale(model);
}