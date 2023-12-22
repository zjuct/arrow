#include "control.h"
#include "defs.h"
#include "glm/gtc/matrix_transform.hpp"
#include "light.hpp"
#include "shader.h"
#include "texturemgr.hpp"

#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

void printVec3(const glm::vec3 &v)
{
    std::cout << v.x << " " << v.y << " " << v.z << std::endl;
}

static Control *control = Control::getInstance();
static UI *ui = UI::getInstance();

extern int BackendMain();

extern std::mutex updateMutex;

void init()
{
    // 初始化glfw，使用OpenGL 3.3
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口，设置OpenGL context
    control->window = glfwCreateWindow(control->wwidth, control->wheight, "Solar System", NULL, NULL);
    if (control->window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    
    glfwSetKeyCallback(control->window, keyCB);
    glfwSetFramebufferSizeCallback(control->window, fbSizeCB);
    glfwSetCursorPosCallback(control->window, mouseMoveCB);
    glfwSetScrollCallback(control->window, scrollCB);
    glfwSetMouseButtonCallback(control->window, mousePressCB);

    glfwSetInputMode(control->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

long long beginTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
int main()
{
    init();
    std::thread backend(BackendMain);
    // 渲染循环
    while (!glfwWindowShouldClose(control->window))
    {

        updateMutex.lock();
        glfwPollEvents();
        
        // std::cout << "BackendMain" << std::endl;
        
        float currenttime = (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() - beginTime) / 1000.0f;
        // float currenttime = glfwGetTime();
        std::cout<<currenttime<<std::endl;
        static int first = 0;
        if (first == 0)
        {
            control->oldTime = currenttime;
            first = 1;
        }
        control->dt = currenttime - control->oldTime;
        control->oldTime = currenttime;

        ui->update();
        if (ui->gstate == GLOBAL_GAME)
        {
            control->players[PLAYER_ID].update(control->dt);
            control->players[ANOTHER_PLAYER_ID].update(control->dt);
            control->camera.updateCamera();
            control->arrowMgr->updateArrow(PLAYER_ID, control->players[PLAYER_ID].getWeaponPos(), glm::normalize(control->camera.Position + control->camera.Front * AIM_DISTANCE - control->players[PLAYER_ID].getWeaponPos()));
            control->arrowMgr->updateArrow(ANOTHER_PLAYER_ID, control->players[ANOTHER_PLAYER_ID].getWeaponPos(), glm::normalize(control->camera.Position + control->camera.Front * AIM_DISTANCE - control->players[ANOTHER_PLAYER_ID].getWeaponPos()));
            updateMutex.unlock();
            updateMutex.lock();
            control->arrowMgr->update(control->dt);
            updateMutex.unlock();
            updateMutex.lock();
            control->candyMgr->update(control->dt);
        }
        updateMutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // std::cout << "fps: " << 1.0f / control->dt << std::endl;
    }
    backend.join();

    glfwTerminate();
    return 0;
}
