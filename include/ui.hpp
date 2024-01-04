#ifndef _UI_HPP
#define _UI_HPP
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include "defs.h"
#include "shader.h"
#include "texturemgr.hpp"

#define SEGMENTS (100)

struct Character {
    GLuint     TextureID;  // 字形纹理ID
    glm::ivec2 Size;       // 字形大大小
    glm::ivec2 Bearing;    // 字形基于基线和起点的位置
    GLuint     Advance;    // 起点到下一个字形起点的距离
};

enum AimState 
{
    AIM_FIRE, AIM_STILL
};

class Aim {
  public:
  Aim() 
  {
  } 
  void draw(Shader* shader);

  void init();
  void update();
  void setState(enum AimState s);

  unsigned int VAO, VBO, EBO;
  enum AimState state = AIM_STILL;
};

class Background {
  public:
  Background() : texname("resource/assets/background/background.png")
  {

  }
  void init();
  void draw(Shader *shader);

  unsigned int VAO, VBO, EBO;
  std::string texname;
};

enum ButtonState {
  BUTTON_ON, BUTTON_OFF
};

class Button {
  public:
  Button()
  {

  }
  Button(glm::vec3 _position, float _width, float _height, bool _has_texture = false, std::string _texname = "", glm::vec3 _color = glm::vec3(1.0f))
  : position(_position), width(_width), height(_height), has_texture(_has_texture), texname(_texname), color(_color), clicked(false), state(BUTTON_OFF)
  {
  }
  
  void init();
  void update();
  void draw(Shader *shader);

  void handleMousePress(int button, int action);
	void handleMouseMove(double xposIn, double yposIn);

  glm::vec3 position;
  float width;
  float height;
  glm::vec3 color;
  bool has_texture;
  std::string texname;
  ButtonState state;
  bool clicked;

  unsigned int VAO, VBO, EBO;
};

class Text {

  std::string content;
  glm::vec3 color;
  float posx;         // 文本框左下角
  float posy;         // 
  float scale;        // based on 0.02

  public:
  Text() {}
  Text(std::string _content, float _posx, float _posy, float _scale, glm::vec3 _color) : content(_content), posx(_posx), posy(_posy), scale(_scale), color(_color) {}

  void init();
  void setText(std::string text);
  void draw(Shader* shader);
  
};

class Rectangle {
    public:
  Rectangle()
  {
  }
  Rectangle(glm::vec3 _position, float _width, float _height, bool _wireFrame, glm::vec3 _color, bool _has_texture = false, std::string _texname = "")
  : position(_position), width(_width), height(_height), has_texture(_has_texture), texname(_texname), color(_color), wireFrame(_wireFrame)
  {
  }
  
  void init();
  void update();
  void draw(Shader *shader);

  void setWidth(float _width)
  {
    this->width = _width;
  }

  float getWidth()
  {
    return width;
  }

  glm::vec3 position;
  float width;
  float height;
  glm::vec3 color;
  bool has_texture;
  std::string texname;
  bool wireFrame;

  unsigned int VAO, VBO, EBO;
};

enum GlobalState {
  GLOBAL_INIT, GLOBAL_GAME
};
class UI {

  public:
  static UI* getInstance();

  void handleMousePress(int button, int action);
	void handleMouseMove(double xposIn, double yposIn);
	void handleKeyInput(int key, int action);
	void handleScroll(double xoffset, double yoffset);

  void init();
  void update();
  void draw(Shader* shader);
  void updateModel();
  void updateBlood();
  void updateExperience();
  void updateLevel();

  void setLevelUp(bool levelUp)
  {
    this->levelUp = levelUp;
  }

  Aim aim;
  Button bt;
  Background bg;
  Text option[3];
  Rectangle blood_1;
  Rectangle blood;
  Rectangle experience_1;
  Rectangle experience;
  Text level;

  bool levelUp;
  bool leftPress;
  bool rightPress;

  GlobalState gstate;
  GlobalState gstate_last;

  private:
	UI();
	UI(const UI&) = delete;
	UI& operator = (const UI&) = delete;

};

// void RenderText(Shader *shader, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
#endif