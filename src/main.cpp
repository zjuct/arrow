#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
#include "control.h"
#include "light.hpp"
#include "texturemgr.hpp"
#include "defs.h"

#include <iostream>
#include <windows.h>

void printVec3(const glm::vec3 &v)
{
    std::cout << v.x << " " << v.y << " " << v.z << std::endl;
}

static Control *control = Control::getInstance();
static UI      *ui      = UI::getInstance();

extern DWORD WINAPI BackendMain(LPVOID lpParameter);



int main()
{
    control->init();
    ui->init();

    CreateThread(NULL, 0, BackendMain, NULL, 0, NULL);
    // 渲染循环
    while (!glfwWindowShouldClose(control->window))
    {
        

		glfwPollEvents();
        
        control->camera.updateCamera();
        
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

        glDepthMask(GL_FALSE);
        skybox_shader->use();
        view = glm::mat4(glm::mat3(view));
        skybox_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));
        skybox_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));

        control->skybox_obj.draw();
        glDepthMask(GL_TRUE);

        ui->draw();

#ifdef SAT_TEST
        control->test.draw(diffuse_shader);
#endif
        glfwSwapBuffers(control->window);

        

        // std::cout << "fps: " << 1.0f / control->dt << std::endl;
    }

    glfwTerminate();
    return 0;
}
