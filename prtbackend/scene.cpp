#include <scene.h>
#include <loader.h>

#include <cassert>
#include <iostream>

Scene::Scene(const char* objfile) {
    LoadObj(objfile, points, normals, albedos);
    assert(points.size() == normals.size() && normals.size() == albedos.size());
    nr_triangles = points.size() / 3;
}

bool Scene::RayTriangleIntersect(const Ray& ray, int idx, Intersect& intersect) const {
    glm::vec3 P0 = points[idx * 3];
    glm::vec3 P1 = points[idx * 3 + 1];
    glm::vec3 P2 = points[idx * 3 + 2];

    glm::vec3 E1 = P1 - P0;
    glm::vec3 E2 = P2 - P0;
    glm::vec3 S = ray.O - P0;
    glm::vec3 S1 = glm::cross(ray.D, E2);
    glm::vec3 S2 = glm::cross(S, E1);

    float det = glm::dot(S1, E1);
    if(det > -1e-8f && det < 1e-8f)
        return false;

    float denom = 1.0f / det;
    intersect.t = denom * glm::dot(S2, E2);
    intersect.bary[1] = denom * glm::dot(S1, S);
    intersect.bary[2] = denom * glm::dot(S2, ray.D);
    intersect.bary[0] = 1.0f - intersect.bary[1] - intersect.bary[2];
    if(intersect.t <= 0.0001f)
        return false;
    for(int i = 0; i < 3; i++) {
        if(intersect.bary[i] < 0.0f || intersect.bary[i] > 1.0f)
            return false;
    }
//    std::cout << "Intersect" << std::endl;
//    std::cout << ray.O.x << " " << ray.O.y << " " << ray.O.z << std::endl;
//    std::cout << ray.D.x << " " << ray.D.y << " " << ray.D.z << std::endl;
//    std::cout << intersect.t << std::endl;
//    std::cout << P0.x << " " << P0.y << " " << P0.z << std::endl;
//    std::cout << P1.x << " " << P1.y << " " << P1.z << std::endl;
//    std::cout << P2.x << " " << P2.y << " " << P2.z << std::endl;
//    std::cout << intersect.bary[0] << " " << intersect.bary[1] << " " << intersect.bary[2] << std::endl;
    return true;
}

bool Scene::RayIntersect(const Ray& ray, Intersect& intersect) const {
    // 暴力遍历所有三角形
    intersect.t = INFINITY;
    Intersect tmp;
    bool inter = false;
    for(int i = 0; i < nr_triangles; i++) {
        if(RayTriangleIntersect(ray, i, tmp)) {
            inter = true;
            if(tmp.t < intersect.t) {
                intersect = (Intersect) {
                    .bary = {tmp.bary[0], tmp.bary[1], tmp.bary[2]},
                    .points = {points[3 * i], points[3 * i + 1], points[3 * i + 2]},
                    .tri_idx = i,
                    .point = ray.O + tmp.t * ray.D,
                    .t = tmp.t,
                };
            }
        }
    }
    return inter;
}