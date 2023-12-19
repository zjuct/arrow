#ifndef GROUND_H
#define GROUND_H
#include "object.hpp"
#include "objLoader.hpp"

class Ground {
  public:
  Ground ();
  ~Ground ();

  void init(const char* objectfile);
  void draw();

  private:
  Scene *obj;
  Object model;
};
#endif