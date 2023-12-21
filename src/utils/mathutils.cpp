#include <mathutils.h>

// 假设circle和obb都处于世界坐标
bool Circle_Obb_intersect2D(const Circle2D& circle, const Obb2D& obb) {
    // 将圆心投影到OBB坐标系上
    glm::vec2 diff = circle.o - obb.c;
    diff = obb.rot * diff;

    // 找到OBB上距圆心最近的那个点的世界坐标
    glm::vec2 clamped = glm::clamp(diff, -obb.ext, obb.ext);
    glm::vec2 closest_global = obb.c + obb.rot * clamped;

    float dist = glm::length(closest_global - circle.o);
    return dist < circle.r;
}

bool CylinderObbIntersect(const Cylinder& cylinder, const Obb& obb) {

}