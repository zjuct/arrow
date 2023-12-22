#include "control.h"
#include "ui.hpp"

  static UI* ui = UI::getInstance();
  static Control* control = Control::getInstance();

/* ============================= class Aim ================================ */

  void Aim::draw(Shader* shader)
  {
    glm::mat4 m = glm::mat4(1.0);
    switch (state) {
      case AIM_STILL:
        
        m = glm::scale(m, glm::vec3(0.05, 0.05, 0.05));
        m = glm::scale(m, glm::vec3((float)WHEIGHT/WWIDTH, 1.0, 1.0)); // 按窗口长宽比缩放
        shader->use();
        shader->setBool("has_texture", false);
        shader->setvec3("color", 1.0, 1.0, 1.0);
        shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));
        break;

      case AIM_FIRE:
        m = glm::scale(m, glm::vec3(0.06, 0.06, 0.06));
        m = glm::scale(m, glm::vec3((float)WHEIGHT/WWIDTH, 1.0, 1.0)); // 按窗口长宽比缩放
        shader->use();
        shader->setBool("has_texture", false);
        shader->setvec3("color", 0.8f, 0.0f, 0.0f);
        shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));
    }

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
    glEnable(GL_DEPTH_TEST);
  }

  void Aim::init()
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

  void Aim::setState(enum AimState s)
  {
    state = s;
  }
/* ================================= class Background ================================ */
  void Background::init()
  {
    float v[20] = {
      -1.0f,  1.0f, 0.0f,  0.0f, 0.0f,
      1.0f,   1.0f, 0.0f,  1.0f, 0.0f,
      -1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
      1.0f,  -1.0f, 0.0f,  1.0f, 1.0f 
    };
    unsigned int idx[6] = {0, 1, 3, 0, 2, 3};

    TextureMgr::getInstance()->load(texname, TEX_2D);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void Background::draw(Shader* shader)
  {
    shader->use();
    shader->setBool("has_texture", true);
    
    int tex = TextureMgr::getInstance()->gettex(texname, TEX_2D);
    if (tex < 0) {
      std::cerr << "[ERROR] No texture." << std::endl;
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);      // 将纹理数据绑定到纹理单元

    glm::vec3 color(1.0f);
    glm::mat4 model(1.0f);
    shader->setvec3fv("color", glm::value_ptr(color));
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(VAO);
    glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glEnable(GL_DEPTH_TEST);
  }
/* ================================= class Button ==================================== */
  
  void Button::init() 
  {
    float v[20];
    v[0] = position.x,          v[1] = position.y,            v[2] = 0.0f,  v[3] = 0.0f,  v[4] = 0.0f;
    v[5] = position.x + width,  v[6] = position.y,            v[7] = 0.0f,  v[8] = 1.0f,  v[9] = 0.0f;
    v[10] = position.x,         v[11] = position.y - height,  v[12] = 0.0f, v[13] = 0.0f, v[14] = 1.0f;
    v[15] = position.x + width, v[16] = position.y - height,  v[17] = 0.0f, v[18] = 1.0f, v[19] = 1.0f;

    int idx[6] = {0, 1, 3, 0, 2, 3};

    TextureMgr::getInstance()->load(texname, TEX_2D);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void Button::handleMousePress(int button, int action)
  {
    if (action == GLFW_PRESS)
    {
      switch (button)
      {
      case GLFW_MOUSE_BUTTON_RIGHT:
        break;
      case GLFW_MOUSE_BUTTON_LEFT:
        std::cerr << "[DEBUG] Left button pressed." << std::endl;
        if (state == BUTTON_ON) clicked = true;
        break;
      }
    }
    if (action == GLFW_RELEASE)
    {
      clicked = false;
    }
  }
	void Button::handleMouseMove(double xposIn, double yposIn)
  {
    float xpos = static_cast<float>(xposIn);
	  float ypos = static_cast<float>(yposIn);
    xpos = xpos / control->wwidth * 2;
    ypos = ypos / control->wheight * 2;
    xpos = xpos - 1;
    ypos = 1 - ypos;

    if (xpos > position.x && xpos < position.x + width &&
        ypos < position.y && ypos > position.y - height)
    {
      state = BUTTON_ON;
      cout << "set button on" << endl;
    }
    else 
    state = BUTTON_OFF;
  }

  void Button::draw(Shader *shader)
  { 
    glm::mat4 m = glm::mat4(1.0f);
    if (state == BUTTON_ON)
    {
      m = glm::scale(m, glm::vec3(1.1f));
    }
    else
    {

    }
    shader->use();
    shader->setBool("has_texture", has_texture);
    if (has_texture) {
      int tex = TextureMgr::getInstance()->gettex(texname, TEX_2D);
      if (tex < 0) {
        std::cerr << "[ERROR] No texture." << std::endl;
      }
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex);      // 将纹理数据绑定到纹理单元
    }

    shader->setvec3fv("color", glm::value_ptr(color));
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(m));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
  }

/* ================================ class UI =================================== */

  UI::UI() : gstate(GLOBAL_INIT)
  {
  }

  UI* UI::getInstance() // 在本文件中声明/找到静态单例，然后返回
  {
    static UI ui;
    return &ui;
  } 

  void UI::handleMousePress(int button, int action)
  {
    for (auto btn : btns)
      btn.handleMousePress(button, action);

    bt.handleMousePress(button, action);

    if (action == GLFW_PRESS)
    {
      switch (button)
      {
      case GLFW_MOUSE_BUTTON_RIGHT:
        std::cerr << "[DEBUG] Right button pressed." << std::endl;
        leftPress = true;
        break;
      case GLFW_MOUSE_BUTTON_LEFT:
        std::cerr << "[DEBUG] Left button pressed." << std::endl;
        if (gstate == GLOBAL_GAME) aim.setState(AimState::AIM_FIRE);
        rightPress = true;
        break;
      }
    }
    else if(action == GLFW_RELEASE)
    { 
      switch (button)
      {
      case GLFW_MOUSE_BUTTON_RIGHT:
        std::cerr << "[DEBUG] Right button released." << std::endl;
        leftPress = false;
        break;
      case GLFW_MOUSE_BUTTON_LEFT:
        std::cerr << "[DEBUG] Left button released." << std::endl;
        rightPress = false;
        aim.setState(AimState::AIM_STILL);
        break;
      }
    }
  }
	void UI::handleMouseMove(double xposIn, double yposIn)
  {
    bt.handleMouseMove(xposIn, yposIn);
  }
	void UI::handleKeyInput(int key, int action)
  {

  }
	void UI::handleScroll(double xoffset, double yoffset)
  {

  }

  void UI::init()
  { 
    gstate = GLOBAL_INIT;
    glfwSetInputMode(control->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    shader = flat_shader;
    aim.init();
    bt = Button(glm::vec3(0.2f, 0.0f, 0.0f), 0.6f, 0.5f, true, "resource/assets/button/btn.png");
    bt.init();
    bg.init();
  }

  void UI::update()
  {
    if (gstate == GLOBAL_GAME)
    {
    }
    else if (gstate == GLOBAL_INIT)
    {
      if (bt.clicked)
      {
        gstate = GLOBAL_GAME;
        glfwSetInputMode(control->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      }
    }

  }

  void UI::draw()
  { 
    shader->use();
    switch (gstate) {
      case GLOBAL_GAME:
        control->ground.draw();
        for (auto& player : control->players)
            player.draw();
        control->arrowMgr->draw();
        control->candyMgr->draw();
        aim.draw(shader); 
        break;
        
      case GLOBAL_INIT:
        bg.draw(shader);
        bt.draw(shader);
      break;
    }
  }