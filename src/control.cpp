#include <control.h>
#include <texturemgr.hpp>

#include <iostream>

static Control* control = Control::getInstance();

Control* Control::getInstance() {
	static Control control;
	return &control;
}

Control::Control() {

}

void Control::init() {
	srand((unsigned)time(NULL));

	// 初始化glfw，使用OpenGL 3.3
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口，设置OpenGL context
	window = glfwCreateWindow(wwidth, wheight, "Solar System", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, keyCB);
	glfwSetFramebufferSizeCallback(window, fbSizeCB);
	glfwSetCursorPosCallback(window, mouseMoveCB);
	glfwSetScrollCallback(window, scrollCB);
	glfwSetMouseButtonCallback(window, mousePressCB);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// 初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}

	// 配置OpenGL
	glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//    glCullFace(GL_BACK);

	// 初始化视口
	glViewport(0, 0, control->wwidth, control->wheight);

	Shader::initShader();

	camera = Camera(glm::vec3(0.465571f, 0.96744f, 2.21652f));

	player.init("resource/assets/player2/player.obj");

	ground.init("resource/assets/scene/scene.obj");

	// 箭测试
	arrowMgr->init("resource/assets/player2/player.obj");
	arrowMgr->bindArrow(1, camera.Position, camera.Front, ARROW_NORMAL);



}

void mousePressCB(GLFWwindow* window, int button, int action, int mods) {
	control->handleMousePress(button, action);
}

void mouseMoveCB(GLFWwindow* window, double xpos, double ypos) {
	control->handleMouseMove(xpos, ypos);
}

void keyCB(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	control->handleKeyInput(key, action);
}

void scrollCB(GLFWwindow* window, double xoffset, double yoffset) {
	control->handleScroll(xoffset, yoffset);
}

void fbSizeCB(GLFWwindow* window, int width, int height) {
	control->wwidth = width;
	control->wheight = height;
	glViewport(0, 0, width, height);
}

void Control::handleMousePress(int button, int action) {
	if (action == GLFW_PRESS)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cerr << "[DEBUG] Right button pressed." << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cerr << "[DEBUG] Left button pressed." << std::endl;
			arrowMgr->fire(1, camera.Position, camera.Front);
			break;
		}
	}
}

void Control::handleMouseMove(double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoffset, yoffset);
}

void Control::pollKeyPress() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, dt);
		player.setState(Player::PLAYER_RUN);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, dt);
		player.setState(Player::PLAYER_RUN);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, dt);
		player.setState(Player::PLAYER_RUN);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, dt);
		player.setState(Player::PLAYER_RUN);
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, dt);
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, dt);
}

void Control::handleKeyInput(int key, int action) {
	if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_W: case GLFW_KEY_A: case GLFW_KEY_S: case GLFW_KEY_D:
			player.setState(Player::PLAYER_STILL);
			break;
		}
	}
}

void Control::handleScroll(double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
