#ifndef _RAY_H
#define _RAY_H

#include <glm/glm.hpp>
#include <obb.h>

struct IntersactPoint {
    bool inter;         // 是否相交
    glm::vec3 p;        // 交点
    glm::vec3 n;        // 交点法线
};

class Ray {
public:
    Ray(const glm::vec3& _o, const glm::vec3& _d)
        : o(_o), d(_d) {

    }

    IntersactPoint intersactWith(const Obb& obb);

private:
    glm::vec3 o;        // 原点
    glm::vec3 d;        // 方向

};


#endif