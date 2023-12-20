#include <sattest.h>

#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

SatTest::SatTest() {
    box1.extends = box2.extends = glm::vec3(0.5f, 0.5f, 0.5f);
    box1.center = glm::vec3(0.0f, 1.0f, -1.0f);
    box2.center = glm::vec3(0.0f, 1.0f, 1.0f);
}

void SatTest::updateKeyBoard(int key) {
    switch(key) {
        case GLFW_KEY_SPACE: control_box1 = ~control_box1;
        case GLFW_KEY_I: control_box1 ? box1.center.z -= 0.05f : box2.center.z -= 0.05f; break;
        case GLFW_KEY_K: control_box1 ? box1.center.z += 0.05f : box2.center.z += 0.05f; break;
        case GLFW_KEY_J: control_box1 ? box1.center.x -= 0.05f : box2.center.x -= 0.05f; break;
        case GLFW_KEY_L: control_box1 ? box1.center.x += 0.05f : box2.center.x += 0.05f; break;
        case GLFW_KEY_U: control_box1 ? box1.center.y -= 0.05f : box2.center.y -= 0.05f; break;
        case GLFW_KEY_O: control_box1 ? box1.center.y += 0.05f : box2.center.y += 0.05f; break;
        case GLFW_KEY_1: control_box1 ? box1.rotate = glm::rotate(glm::mat4(box1.rotate), 0.05f, glm::vec3(1.0f, 0.0f, 0.0f)) 
            : box2.rotate = glm::rotate(glm::mat4(box2.rotate), 0.05f, glm::vec3(1.0f, 0.0f, 0.0f));
            break;
        case GLFW_KEY_2: control_box1 ? box1.rotate = glm::rotate(glm::mat4(box1.rotate), 0.05f, glm::vec3(0.0f, 1.0f, 0.0f)) 
            : box2.rotate = glm::rotate(glm::mat4(box2.rotate), 0.05f, glm::vec3(0.0f, 1.0f, 0.0f));
            break;
        case GLFW_KEY_3: control_box1 ? box1.rotate = glm::rotate(glm::mat4(box1.rotate), 0.05f, glm::vec3(0.0f, 0.0f, 1.0f)) 
            : box2.rotate = glm::rotate(glm::mat4(box2.rotate), 0.05f, glm::vec3(0.0f, 0.0f, 1.0f));
            break;

    }
    checkCollision();
}

void SatTest::draw(Shader* shader) {
    box1.draw(shader);
    box2.draw(shader);
}

void SatTest::checkCollision() {
    if(box1.intersectWith(box2)) {
        box1.material.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
        box2.material.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
    } else {
        box1.material.albedo = glm::vec3(0.0f, 0.0f, 1.0f);
        box2.material.albedo = glm::vec3(0.0f, 0.0f, 1.0f);
    }
}