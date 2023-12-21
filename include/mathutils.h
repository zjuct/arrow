#ifndef _MATHUTILS_H
#define _MATHUTILS_H

#include <glm/glm.hpp>

#include <obb.h>

struct Circle2D {
    glm::vec2 o;
    glm::vec2 r;
};

struct Obb2D {
    glm::vec2 c;
    glm::mat2 rot;
    glm::vec2 ext;
};

struct Cylinder {

};

struct IntersectResult {
    bool inter;
    glm::vec3 p;
    glm::vec3 n;
};

bool Circle_Obb_intersect2D(const Circle2D& circle, const Obb2D& obb);

#endif