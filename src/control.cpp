#include "defs.h"
#include <camera.hpp>
#include <control.h>
#include <texturemgr.hpp>

#include <iostream>
#include <windows.h>

int current_player = 1;

static Control *control = Control::getInstance();
static UI *ui = UI::getInstance();

Control *Control::getInstance()
{
    static Control control;
    return &control;
}

Control::Control()
{
}

void Control::init()
{
    srand((unsigned)time(NULL));

    // 初始化glfw，使用OpenGL 3.3
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口，设置OpenGL context
    window = glfwCreateWindow(wwidth, wheight, "Solar System", NULL, NULL);
    if (window == NULL)
    {
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
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
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

	skybox = Box(glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 skybox_model = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
    skybox_obj = Object(OBJECT_BOX, &skybox, skybox_shader, skybox_model);
    skybox_obj.material.skybox_texname = "resource/assets/skybox_zjg";

    Player player1;
    player1.init("resource/assets/player2/player.obj", glm::vec3(-1.0f, 0.0f, 0.0f));
    players.push_back(player1);
    Player player2;
    player2.init("resource/assets/player2/player.obj", glm::vec3(-1.0f, 0.0f, 0.0f));
    players.push_back(player2);
    camera.follow(&players[PLAYER_ID]);

    ground.init("resource/assets/scene/scene.obj");

    // 箭
    arrowMgr->init("resource/assets/weapon/knife.obj");
    arrowMgr->bindArrow(PLAYER_ID, ARROW_NORMAL);
    arrowMgr->bindArrow(ANOTHER_PLAYER_ID, ARROW_NORMAL);

    // 道具
    candyMgr->init("resource/assets/weapon/knife.obj");

#ifdef SAT_TEST
    test.init();
#endif
}

void mousePressCB(GLFWwindow *window, int button, int action, int mods)
{
    if (ui->gstate == GLOBAL_GAME)
        control->handleMousePress(button, action);
    ui->handleMousePress(button, action);
}

void mouseMoveCB(GLFWwindow *window, double xpos, double ypos)
{
    if (ui->gstate == GLOBAL_GAME)
        control->handleMouseMove(xpos, ypos);
    ui->handleMouseMove(xpos, ypos);
}

void keyCB(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (ui->gstate == GLOBAL_GAME)
        control->handleKeyInput(key, action);
}

void scrollCB(GLFWwindow *window, double xoffset, double yoffset)
{
    if (ui->gstate == GLOBAL_GAME)
        control->handleScroll(xoffset, yoffset);
}

void fbSizeCB(GLFWwindow *window, int width, int height)
{
    control->wwidth = width;
    control->wheight = height;
    glViewport(0, 0, width, height);
}

void Control::handleMousePress(int button, int action)
{
    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_RIGHT:
            std::cerr << "[DEBUG] Right button pressed." << std::endl;
            break;
        case GLFW_MOUSE_BUTTON_LEFT:
            std::cerr << "[DEBUG] Left button pressed." << std::endl;
            leftMousePress = true;
            break;
        }
    }
    else if (action == GLFW_RELEASE)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_RIGHT:
            std::cerr << "[DEBUG] Right button released." << std::endl;
            break;
        case GLFW_MOUSE_BUTTON_LEFT:
            std::cerr << "[DEBUG] Left button released." << std::endl;
            leftMousePress = false;
            // arrowMgr->fire(PLAYER_ID);
            players[PLAYER_ID].fire();
            break;
        }
    }
}

void Control::handleMouseMove(double xposIn, double yposIn)
{
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
    // camera.ProcessMouseMovement(xoffset, yoffset);
    players[PLAYER_ID].processMouseMovement(xoffset, yoffset);
}

void Control::pollKeyPress()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void Control::handleKeyInput(int key, int action)
{
#ifdef SAT_TEST
    if (action == GLFW_PRESS)
    {
        test.updateKeyBoard(key);
    }
#endif
    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_W)
        {
            frontPress = false;
        }
        if (key == GLFW_KEY_S)
        {
            backPress = false;
        }
        if (key == GLFW_KEY_A)
        {
            leftPress = false;
        }
        if (key == GLFW_KEY_D)
        {
            rightPress = false;
        }
        if (key == GLFW_KEY_UP)
        {
            another_frontPress = false;
        }
        if (key == GLFW_KEY_DOWN)
        {
            another_backPress = false;
        }
        if (key == GLFW_KEY_LEFT)
        {
            another_leftPress = false;
        }
        if (key == GLFW_KEY_RIGHT)
        {
            another_rightPress = false;
        }
    }
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_SPACE)
            players[PLAYER_ID].jump();
        if (key == GLFW_KEY_W)
        {
            frontPress = true;
        }
        if (key == GLFW_KEY_S)
        {
            backPress = true;
        }
        if (key == GLFW_KEY_A)
        {
            leftPress = true;
        }
        if (key == GLFW_KEY_D)
        {
            rightPress = true;
        }
        if (key == GLFW_KEY_UP)
        {
            another_frontPress = true;
        }
        if (key == GLFW_KEY_DOWN)
        {
            another_backPress = true;
        }
        if (key == GLFW_KEY_LEFT)
        {
            another_leftPress = true;
        }
        if (key == GLFW_KEY_RIGHT)
        {
            another_rightPress = true;
        }
        if (key == GLFW_KEY_M)
        {
            current_player = (current_player ^ 1);
            camera.follow(&players[PLAYER_ID]);
        }
    }
}

void Control::handleScroll(double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


DWORD WINAPI BackendMain(LPVOID lpParameter)
{
    for (;;)
    {
        std::cout << "BackendMain" << std::endl;

        float currenttime = glfwGetTime();
		static int first = 0;
        if(first == 0)
        {
            control->oldTime = currenttime;
            first = 1;
        }
        control->dt = currenttime - control->oldTime;
        control->oldTime = currenttime;
        if (ui->gstate == GLOBAL_GAME)
        {
        control->players[PLAYER_ID].update(control->dt);
        control->players[ANOTHER_PLAYER_ID].update(control->dt);
        control->arrowMgr->updateArrow(PLAYER_ID, control->players[PLAYER_ID].getWeaponPos(), glm::normalize(control->camera.Position + control->camera.Front * AIM_DISTANCE - control->players[PLAYER_ID].getWeaponPos()));
        control->arrowMgr->updateArrow(ANOTHER_PLAYER_ID, control->players[ANOTHER_PLAYER_ID].getWeaponPos(), glm::normalize(control->camera.Position + control->camera.Front * AIM_DISTANCE - control->players[ANOTHER_PLAYER_ID].getWeaponPos()));
        control->arrowMgr->update(control->dt);
        control->candyMgr->update(control->dt);
        if (control->leftMousePress)
            control->leftPressTime += control->dt;
        else
            control->leftPressTime = 0.0f;

            control->pollKeyPress();
        }

    }
}