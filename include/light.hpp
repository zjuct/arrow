#ifndef _LIGHT_HPP_
#define _LIGHT_HPP_

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shader.h"
#include "box.hpp"
#include "object.hpp"
#include "glm/glm.hpp"
#include <cstdio>

class Light {
public:
    virtual void configShader(Shader* shader) = 0;
//    virtual void draw() = 0;
    virtual void update(float dt) = 0;
};

class DirLight: public Light {
public:
    DirLight(glm::vec3 dir = glm::vec3(0.0f, -1.0f, -1.0f), glm::vec3 a = glm::vec3(0.05), glm::vec3 d = glm::vec3(0.5), glm::vec3 s = glm::vec3(0.5))
        : direction(dir), ambient(a), diffuse(d), specular(s) {

    }

    void configShaderShadowMap(Shader* shader) {
        shader->use();
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(-10.0f * direction, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        shader->setmat4fv("lightSpaceMatrix", GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    }

    void configShader(Shader* shader) {
//        configShaderShadowMap(shader);
        shader->use();
        shader->setBool("dirLight.enable", true);
        shader->setvec3fv("dirLight.direction", glm::value_ptr(direction));
        shader->setvec3fv("dirLight.ambient", glm::value_ptr(ambient));
        shader->setvec3fv("dirLight.diffuse", glm::value_ptr(diffuse));
        shader->setvec3fv("dirLight.specular", glm::value_ptr(specular));
    }

    void update(float dt) {}

    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float near_plane = 1.0f;
    float far_plane = 25.0f;
};

class PointLight: public Light {
public:
    PointLight(glm::vec3 pos = glm::vec3(0.0f, 100.0f, 0.0f), float c = 1.0f, float l = 0.0014f, float q = 0.000007f, 
        glm::vec3 a = glm::vec3(0.5f), glm::vec3 d = glm::vec3(0.5f), glm::vec3 s = glm::vec3(0.5f), Shader* shader = nullptr, bool hasBox = false)
        : position(pos), constant(c), linear(l), quadratic(q), ambient(a), diffuse(d), specular(s), shader(shader) {
        if(hasBox) {
            setupBox();
        }
    }

    void configShader(Shader* shader) {
        shader->use();
        shader->setBool("pointLight.enable", true);
        shader->setvec3fv("pointLight.position", glm::value_ptr(position));
        shader->setFloat("pointLight.constant", constant);
        shader->setFloat("pointLight.linear", linear);
        shader->setFloat("pointLight.quadratic", quadratic);
        shader->setvec3fv("pointLight.ambient", glm::value_ptr(ambient));
        shader->setvec3fv("pointLight.diffuse", glm::value_ptr(diffuse));
        shader->setvec3fv("pointLight.specular", glm::value_ptr(specular));
    }

//    void draw() {
//        if(box) {
//            box->draw();
//        }
//    }

    void update(float dt) {
        float omega = 0.1f;
        float delta = dt * omega;
        glm::mat4 m(1.0f);
        m = glm::rotate(m, delta, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec4 p(position.x, position.y, position.z, 1.0f);
        p = m * p;
        position = glm::vec3(p.x, p.y, p.z);
        box->mulleft(m);
    }

private:
    void setupBox() {
        Shape* box_shape = new Box(glm::vec3(1.0f, 1.0f, 1.0f));
        if(box_shape) {
            glm::mat4 model(1.0f);
            model = glm::translate(model, position);
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            box = new Object(OBJECT_BOX, box_shape, shader, model);
        }
    }


    glm::vec3 position;
    float constant;
    float linear;
    float quadratic;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    Object* box;
    Shader* shader;
};

class PBRDirLight: public Light {
public:
    PBRDirLight(const glm::vec3& dir, const glm::vec3& _intensity)
        : direction(dir), intensity(_intensity) {

    } 

    void configShader(Shader* shader) {
        shader->use();
        shader->setBool("dirLight.enable", true);
        shader->setvec3fv("dirLight.direction", glm::value_ptr(direction));
        shader->setvec3fv("dirLight.intensity", glm::value_ptr(intensity));
    }

    void draw() {}
    void update(float dt) {}

private:
    glm::vec3 direction;
    glm::vec3 intensity;
};

class PBRPointLight: public Object {
public:
    PBRPointLight(int _id, const glm::vec3& p, const glm::vec3& i, bool _show)
        : position(p), intensity(i), id(_id) {
        shape = new Box(glm::vec3(1.0f, 1.0f, 1.0f));
        type = OBJECT_LIGHT;
        show = _show;

        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        setModel(model);
        setModel_noscale(glm::translate(glm::mat4(1.0f), position));
    }

    void configShader(Shader* shader) {
        shader->use();
        shader->setBool("pointLight[" + std::to_string(id) + "].enable", true);
        glm::vec4 p4(position.x, position.y, position.z, 1.0f);
        p4 = gmodel * p4;
        glm::vec3 p3(p4.x, p4.y, p4.z);
        shader->setvec3fv("pointLight[" + std::to_string(id) + "].position", glm::value_ptr(p3));
        shader->setvec3fv("pointLight[" + std::to_string(id) + "].intensity", glm::value_ptr(intensity));
    }
    
    void update(float dt) {

    }
    glm::vec3 position;
    glm::vec3 intensity;
    int id;
};


#endif