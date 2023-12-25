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

  Aim aim;
  Button bt;
  Background bg;
  std::vector<Button> btns;
  bool leftPress;
  bool rightPress;

  GlobalState gstate;

  private:
	UI();
	UI(const UI&) = delete;
	UI& operator = (const UI&) = delete;

};
#endif