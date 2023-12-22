#include "ground.hpp"
#include "shader.h"

Ground::Ground() {}
Ground::~Ground() {}

void Ground::init(const char* objectfile) {
  obj = Scene::LoadObj(objectfile);
  model = Object(OBJECT_NONE, nullptr, nullptr, glm::mat4(1.0),
                  nullptr, false);
  for (auto& mesh : obj->getMesh()) {
    Object* node = new Object(OBJECT_MESH, &mesh, player_shader);
    model.addChild(node);
  }
}

void Ground::draw() {
  model.draw();
}

void Ground::updateModel() {
  glm::mat4 mod = glm::mat4(1.0);
  mod = glm::translate(mod, glm::vec3(0.0f, -0.131677f, 0.0f));
  model.setModel(mod);
  model.setModel_noscale(mod);
}