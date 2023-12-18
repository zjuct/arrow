#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
#include "control.h"
#include "light.hpp"
#include "texturemgr.hpp"

#include <iostream>

void printVec3(const glm::vec3 &v)
{
    std::cout << v.x << " " << v.y << " " << v.z << std::endl;
}

static Control *control = Control::getInstance();

int main()
{
    control->init();

    Box skybox(glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 skybox_model = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
    Object skybox_obj(OBJECT_BOX, &skybox, skybox_shader, skybox_model);
    skybox_obj.material.skybox_texname = "resource/assets/skybox_zjg";

    // 渲染循环
    while (!glfwWindowShouldClose(control->window))
    {

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置view和projection矩阵
        default_shader->use();
        glm::mat4 projection = glm::perspective(glm::radians(control->camera.Zoom), (float)control->wwidth / (float)control->wheight, 0.1f, 100.0f);
        default_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        glm::mat4 view = control->camera.GetViewMatrix();
        default_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));
        default_shader->setvec3fv("viewPos", glm::value_ptr(control->camera.Position));
        default_shader->setBool("dirLight.enable", false);
        default_shader->setBool("pointLight.enable", false);

        diffuse_shader->use();
        diffuse_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));
        diffuse_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));

        glDepthMask(GL_FALSE);
        skybox_shader->use();
        view = glm::mat4(glm::mat3(view));
        skybox_shader->setmat4fv("view", GL_FALSE, glm::value_ptr(view));
        skybox_shader->setmat4fv("projection", GL_FALSE, glm::value_ptr(projection));

        skybox_obj.draw();
        glDepthMask(GL_TRUE);
        control->player.draw();

        glfwSwapBuffers(control->window);
        glfwPollEvents();

        float currenttime = glfwGetTime();
        control->dt = currenttime - control->oldTime;
        control->oldTime = currenttime;
        control->player.update(control->dt);

        control->pollKeyPress();
        std::cout << "pos:";
        printVec3(control->camera.Position);

        std::cout << "Front:";
        printVec3(control->camera.Front);
    }

    glfwTerminate();
    return 0;
}