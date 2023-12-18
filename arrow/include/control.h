#ifndef _CONTROL_H
#define _CONTROL_H

/**
* �����û����룬��������ƶ��ͳ�������
*/ 
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera.hpp>
#include <player.h>

void mousePressCB(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCB(GLFWwindow* window, double xpos, double ypos);
void keyCB(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCB(GLFWwindow* window, double xoffset, double yoffset);
void fbSizeCB(GLFWwindow* window, int width, int height);

class Control {
public:
	// ����ģʽ
	static Control* getInstance();
	void init();

	void handleMousePress(int button, int action);
	void handleMouseMove(double xposIn, double yposIn);
	void handleKeyInput(int key, int action);
	void handleScroll(double xoffset, double yoffset);


	// ���
	Camera camera;

	// ���(��Player�б���ͬ��)
	Player player;

	// ����
	GLFWwindow* window = nullptr;
	float lastX = 0.0f;
	float lastY = 0.0f;
	bool firstMouse = true;
	double dt = 0.0;		// ���ʱ��
	double oldTime = 0.0;	// �ϴθ���ʱ��
	int wwidth = 800;
	int wheight = 600;

	int fps = 0;

	bool end = false;

private:
	Control();
	Control(const Control&) = delete;
	Control& operator = (const Control&) = delete;
};

#endif