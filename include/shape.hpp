#ifndef _SHAPE_HPP_
#define _SHAPE_HPP_

/*
Shape泛指模型空间下的几何体，其与位姿Pose共同构成一个具体的模型
Object = Shape + Model matrix
下面的类型都属于Shape
    - Mesh
    - Box(其实可以把Box归到Mesh里，但为了方便使用，单独搞一个)
Shape并不包含层次结构，层次结构定义在Object中
*/

#include <shader.h>
#include <glm/glm.hpp>

#include <obb.h>

#include <vector>

struct vertex_t {
    vertex_t (const glm::vec3& v): vertices(v) {}
    vertex_t (const glm::vec3& v, const glm::vec2& t): vertices(v), texCoords(t) {}
    vertex_t (const glm::vec3& v, const glm::vec3 n, const glm::vec2& t): vertices(v), normals(n), texCoords(t) {}
    glm::vec3 vertices;
    glm::vec3 normals;
    glm::vec2 texCoords;
};

class Shape {
public:
    virtual void draw(Shader* shader) = 0;
    virtual Obb* getObb() = 0;
};

extern void calcVNormal(std::vector<vertex_t>& v, const std::vector<unsigned int>& indices);

#endif