#ifndef _UI_HPP
#define _UI_HPP
#include "defs.h"
#include "shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

#define SEGMENTS (100)

class Aim {
  public:
  Aim() 
  {
  } 
  void draw(Shader* shader)
  {
    
    glm::mat4 m = glm::mat4(1.0);
    m = glm::scale(m, glm::vec3(0.05, 0.05, 0.05));
    m = glm::scale(m, glm::vec3((float)WHEIGHT/WWIDTH, 1.0, 1.0)); // 按窗口长宽比缩放
    shader->use();
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, SEGMENTS);

    m = glm::mat4(1.0);
    m = glm::scale(m, glm::vec3(0.04, 0.04, 0.04));
    m = glm::scale(m, glm::vec3((float)WHEIGHT/WWIDTH, 1.0, 1.0)); // 按窗口长宽比缩放
    shader->use();
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));

    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, SEGMENTS);
  }

  void init()
  {
    genCircle();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, circle.size() * sizeof(float), &circle[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void genCircle()
  {
    for (int i = 0; i < SEGMENTS; ++i) {
      float x = cos(2 * M_PIf * i / SEGMENTS);
      float z = 0;
      float y = sin(2 * M_PIf * i / SEGMENTS);
      circle.push_back(x);
      circle.push_back(y);
      circle.push_back(z);
    }
    for (int i = 0; i < circle.size(); i += 3) {
      for (int j = 0; j < 3; ++j)
       std::cout << circle[i+j] << " ";
      std::cout << std::endl;
    }
  }

  unsigned int VAO, VBO;
  std::vector<float> circle;
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
  private:
  Shader *shader;
  Aim aim;

};
#endif