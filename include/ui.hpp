#ifndef _UI_HPP
#define _UI_HPP
#include "defs.h"
#include "shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

#define SEGMENTS (100)
enum AimState 
{
    AIM_FIRE, AIM_STILL
};

class Aim {
  public:
  Aim() 
  {
  } 
  void draw(Shader* shader)
  {
    glm::mat4 m = glm::mat4(1.0);
    switch (state) {
      case AIM_STILL:
        
        m = glm::scale(m, glm::vec3(0.05, 0.05, 0.05));
        m = glm::scale(m, glm::vec3((float)WHEIGHT/WWIDTH, 1.0, 1.0)); // 按窗口长宽比缩放
        shader->use();
        shader->setvec3("color", 1.0, 1.0, 1.0);
        shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));
        break;

      case AIM_FIRE:
        m = glm::scale(m, glm::vec3(0.06, 0.06, 0.06));
        m = glm::scale(m, glm::vec3((float)WHEIGHT/WWIDTH, 1.0, 1.0)); // 按窗口长宽比缩放
        shader->use();
        shader->setvec3("color", 0.8f, 0.0f, 0.0f);
        shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
  }

  void init()
  {
    float vertices[72] = {
      -0.4, 1.0, 0.0,    -0.3, 1.0, 0.0,    0.3, 1.0, 0.0,    0.4, 1.0, 0.0,
      -1.0, 0.4, 0.0,    -0.4, 0.4, 0.0,    0.4, 0.4, 0.0,    1.0, 0.4, 0.0,
      -1.0, 0.3, 0.0,    -0.3, 0.3, 0.0,    0.3, 0.3, 0.0,    1.0, 0.3, 0.0,
      -0.4, -1.0, 0.0,    -0.3, -1.0, 0.0,    0.3, -1.0, 0.0,    0.4, -1.0, 0.0,
      -1.0, -0.4, 0.0,    -0.4, -0.4, 0.0,    0.4, -0.4, 0.0,    1.0, -0.4, 0.0,
      -1.0, -0.3, 0.0,    -0.3, -0.3, 0.0,    0.3, -0.3, 0.0,    1.0, -0.3, 0.0,
    };

    int indices[48] = {
      8, 9, 4,    4, 9, 5,    5, 9, 0,    0, 9, 1,
      2, 10, 3,   3, 10, 6,   6, 10, 7,   7, 10, 11,
      20, 21, 16, 16, 21, 17, 17, 21, 12, 12, 21, 13,
      14, 22, 15, 15, 22, 18, 18, 22, 19, 19, 22, 23
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  // void genCircle()
  // {
  //   for (int i = 0; i < SEGMENTS; ++i) {
  //     float x = cos(2 * M_PIf * i / SEGMENTS);
  //     float z = 0;
  //     float y = sin(2 * M_PIf * i / SEGMENTS);
  //     circle.push_back(x);
  //     circle.push_back(y);
  //     circle.push_back(z);
  //   }
  //   for (int i = 0; i < circle.size(); i += 3) {
  //     for (int j = 0; j < 3; ++j)
  //      std::cout << circle[i+j] << " ";
  //     std::cout << std::endl;
  //   }
  // }

  void setState(enum AimState s)
  {
    state = s;
  }

  unsigned int VAO, VBO, EBO;
  enum AimState state = AIM_STILL;
  // std::vector<float> circle;
};
class UI {
  public:
  UI() {}
  ~UI() {}

  void init()
  { 
    shader = flat_shader;
    aim.init();
  }

  void draw()
  { 
    shader->use();
    aim.draw(shader);
  }

  Shader *shader;
  Aim aim;

};
#endif