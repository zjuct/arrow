#ifndef _SCENE_H
#define _SCENE_H

#include <vector>
#include <array>

#include <glm/glm.hpp>

struct Ray {
    Ray(const glm::vec3& o, const glm::vec3& d): O(o), D(d) {}
    glm::vec3 O;
    glm::vec3 D;
};

struct Intersect {
    std::array<float, 3> bary;
    std::array<glm::vec3, 3> points;
    int tri_idx;

    glm::vec3 point;
    float t;
};

class Scene {
public:
    Scene(const char* objfile);
    bool RayIntersect(const Ray& ray, Intersect& intersect) const;

    // points, normals, albedos都是3个元素一组，构成一个三角形
    std::vector<glm::vec3> points;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> albedos;
    int nr_triangles;

private:
    bool RayTriangleIntersect(const Ray& ray, int idx, Intersect& intersect) const;
};

#endif