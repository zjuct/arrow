#ifndef _CONTROL_H
#define _CONTROL_H

/**
* 处理用户输入，控制相机移动和场景更新
*/ 
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera.hpp>
#include <player.h>
#include <arrow.hpp>
#include <ctime>
#include <vector>

void mousePressCB(GLFWwindow* window, int button, int action, int mods);
void mouseMoveCB(GLFWwindow* window, double xpos, double ypos);
void keyCB(GLFWwindow* window, int key, int scancode, int action, int mods);
void scrollCB(GLFWwindow* window, double xoffset, double yoffset);
void fbSizeCB(GLFWwindow* window, int width, int height);

class Control {
public:
	// 单例模式
	static Control* getInstance();
	void init();

	void handleMousePress(int button, int action);
	void handleMouseMove(double xposIn, double yposIn);
	void handleKeyInput(int key, int action);
	void handleScroll(double xoffset, double yoffset);
	void pollKeyPress();


	// 相机
	Camera camera;

	// 玩家(跟Player中保持同步)
	Player player;

	// 箭
	ArrowManager arrowMgr;


	// 窗口
	GLFWwindow* window = nullptr;
	float lastX = 0.0f;
	float lastY = 0.0f;
	bool firstMouse = true;
	double dt = 0.0;		// 间隔时间
	double oldTime = 0.0;	// 上次更新时间
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