#include "defs.h"
#include <camera.hpp>
#include <control.h>
#include <texturemgr.hpp>

#include <chrono>
#include <iostream>
#include <mutex>
#include <sync.hpp>
#include <winsock2.h>

int current_player = DEFAULT_PLAYER_ID;

static Control *control = Control::getInstance();
static UI *ui = UI::getInstance();

extern SOCKET sock;

Control *Control::getInstance()
{
    static Control control;
    return &control;
}

Control::Control()
{
}

bool backendinitfin = false;
bool gladinit = false;

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
    skybox_obj.material.skybox_texname = "resource/assets/CornellBox";

    // Player player1;
    // player1.init("resource/assets/player2/player.obj", glm::vec3(-1.0f, 0.0f, 0.0f));
    // players.push_back(player1);
    // players[0] = player1;
    // Player player2;
    // player2.init("resource/assets/player2/player.obj", glm::vec3(-2.0f, 0.0f, 0.0f));
    // players.push_back(player2);
    // players[1] = player2;
    // Player player3;
    // player3.init("resource/assets/player2/player.obj", glm::vec3(-3.0f, 0.0f, 0.0f));
    // players.push_back(player3);
    players[PLAYER_ID].init(PLAYER_OBJECT_PATH, glm::vec3(-1.0f, 0.0f, 0.0f), PLAYER_ID);
    camera.follow(PLAYER_ID, &players);
    std::cout << "[DEBUG] Player " << PLAYER_ID << " initialized." << std::endl;

    ground.init("resource/assets/scene/scene.obj");

    // 箭
    arrowMgr->init("resource/assets/weapon/knife.obj");
    arrowMgr->bindArrow(PLAYER_ID, ARROW_NORMAL);
    arrowMgr->arrows[arrowMgr->arrowSetting[PLAYER_ID]].isReflect = true;
    arrowMgr->load(PLAYER_ID);
    // arrowMgr->bindArrow(ANOTHER_PLAYER_ID, ARROW_NORMAL);

    // 道具
    candyMgr->init("resource/assets/candy/candy.obj");

#ifdef SAT_TEST
    test.init();
#endif
    grid.init(ground.getModel().getChildren(), 1.0f);

    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // 绑定depthMap到depthMapFBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    gladinit = true;
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
    {
        control->handleKeyInput(key, action);
        ui->handleKeyInput(key, action);
    }
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
            camera.follow(current_player, &players);
        }
        static int cnt_1 = 0;
        if (key == GLFW_KEY_1)
        {
            if (players[PLAYER_ID].state == Player::PLAYER_DEAD)
            {
                ++cnt_1;
                if (cnt_1 >= 5)
                {
                    players[PLAYER_ID].rebirth();
                    // FuncSyncPackage funcSyncPackage = FuncSyncPackage(FUNC_PLAYER_REBIRTH, &PLAYER_ID);
                    // funcSyncPackage.send(sock);
                    cnt_1 = 0;
                }
            }
        }
    }
}

void Control::handleScroll(double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void Control::initNewPlayer()
{
    for (auto &player : players)
    {
        if (player.second.inited == 0)
        {
            player.second.init(PLAYER_OBJECT_PATH, player.second.position, player.second.id);
            std::cout << "[DEBUG] Player " << player.second.id << " initialized." << std::endl;
        }
    }
}

std::mutex updateMutex;

auto oldtime = std::chrono::system_clock::now();
auto newtime = std::chrono::system_clock::now();

int init = 0;

int FrontendMain()
{

    control->init();
    ui->init();

    init = 1;

    // glfwMakeContextCurrent(control->window);
    while (!glfwWindowShouldClose(control->window))
    {

        newtime = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = newtime - oldtime;
        oldtime = newtime;
        float dt = elapsed_seconds.count();

        // 锁定，将当前状态保存到局部
        updateMutex.lock();
        control->initNewPlayer();
        ui->updateModel();

        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, control->dirLight.near_plane, control->dirLight.far_plane);
        lightView = glm::lookAt(-10.0f * control->dirLight.direction, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

        int wwidth = control->wwidth;
        int wheight = control->wheight;

        glm::mat4 projection = glm::perspective(glm::radians(control->camera.Zoom), (float)control->wwidth / (float)control->wheight, 0.1f, 100.0f);
        glm::mat4 view = control->camera.GetViewMatrix();
        glm::vec3 viewPos = control->camera.Position;

        updateMutex.unlock();

        // 用局部状态配置着色器参数
        shadowmap_shader->use();
        shadowmap_shader->setmat4fv("lightSpaceMatrix", GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        segment_shader->use();
        segment_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        segment_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));

#if PRT_ENABLE
        prt_shader->use();
        prt_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        prt_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));
        prt_shader->setmat3fv("PrecomputeL[0]", GL_FALSE, control->ground.obj->getSHL()[0].data());
        prt_shader->setmat3fv("PrecomputeL[1]", GL_FALSE, control->ground.obj->getSHL()[1].data());
        prt_shader->setmat3fv("PrecomputeL[2]", GL_FALSE, control->ground.obj->getSHL()[2].data());
#endif

        glm::mat4 sky_view = glm::mat4(glm::mat3(view));
        skybox_shader->use();
        skybox_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        skybox_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(sky_view));


        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#if SHADOW_ENABLE
        // 渲染阴影贴图
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, control->depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        ui->draw(shadowmap_shader);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

        // 用深度贴图渲染阴影
        glViewport(0, 0, wwidth, wheight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDepthMask(GL_FALSE);
        control->skybox_obj.draw(skybox_shader);
        glDepthMask(GL_TRUE);

#if SHADOW_ENABLE
        shadow_shader->use();
        shadow_shader->setBool("dirLight.enable", false);
        shadow_shader->setBool("pointLight.enable", false);
        shadow_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        shadow_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));
        shadow_shader->setvec3fv("viewPos", glm::value_ptr(viewPos));
        shadow_shader->setmat4fv("lightSpaceMatrix", GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        shadow_shader->setInt("shadowMap", 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, control->depthMap);
        shadow_shader->setBool("PCF_enable", false);
#if PCF_ENABLE
        shadow_shader->setBool("PCF_enable", true);
#endif
        control->dirLight.configShader(shadow_shader); // TODO: 互斥
        ui->draw(shadow_shader);
#else
        default_shader->use();
        default_shader->setBool("dirLight.enable", false);
        default_shader->setBool("pointLight.enable", false);
        default_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        default_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));
        default_shader->setvec3fv("viewPos", glm::value_ptr(viewPos));
        control->dirLight.configShader(default_shader);
        ui->draw(default_shader);
#endif

        glfwSwapBuffers(control->window);
    }
    return 0;
}