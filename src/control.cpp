#include "defs.h"
#include <camera.hpp>
#include <control.h>
#include <texturemgr.hpp>

#include <iostream>
#include <mutex>
#include <chrono>

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

    
    glfwMakeContextCurrent(window);

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

std::mutex updateMutex;

auto oldtime = std::chrono::system_clock::now();
auto newtime = std::chrono::system_clock::now();

int BackendMain()
{
	
    control->init();
    ui->init();

    // glfwMakeContextCurrent(control->window);
    while (!glfwWindowShouldClose(control->window))
    {
        newtime = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = newtime - oldtime;
        oldtime = newtime;
        float dt = elapsed_seconds.count();
        // std::cout<<"fps:"<<1.0f/dt<<std::endl;

        updateMutex.lock();
		// std::cout << "BackendMain" << std::endl;
        ui->updateModel();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置view和projection矩阵
        default_shader->use();
        glm::mat4 projection = glm::perspective(glm::radians(control->camera.Zoom), (float)control->wwidth / (float)control->wheight, 0.1f, 100.0f);
        default_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        glm::mat4 view = control->camera.GetViewMatrix();
        default_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));
        default_shader->setvec3fv("viewPos", glm::value_ptr(control->camera.Position));
        // default_shader->setBool("dirLight.enable", true);
        // default_shader->setBool("pointLight.enable", true);
        control->dirLight.configShader(default_shader);
        // control->pointLight.configShader(default_shader);

        diffuse_shader->use();
        diffuse_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        diffuse_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));

        segment_shader->use();
        segment_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        segment_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));

        skybox_shader->use();
        view = glm::mat4(glm::mat3(view));
        skybox_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));
        skybox_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));

		// std::cout<<"draw"<<std::endl;
        updateMutex.unlock();

        glDepthMask(GL_FALSE);
        control->skybox_obj.draw();
        glDepthMask(GL_TRUE);

        ui->draw();

#ifdef SAT_TEST
        control->test.draw(diffuse_shader);
#endif
        glfwSwapBuffers(control->window);
    }
    return 0;
}