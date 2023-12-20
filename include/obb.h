#ifndef _OBB_H
#define _OBB_H

#include <shader.h>
#include <material.h>

#include <glm/glm.hpp>

#include <vector>

class Obb {
public:
    static Obb obbgen(const std::vector<glm::vec3>& vertices, bool ground_parallel = true);


    Obb(const glm::vec3& center = glm::vec3(0.0f), const glm::vec3& extends = glm::vec3(1.0f),
        const glm::mat3& rotate = glm::mat3(1.0f));

    void init();

    glm::vec3 center;
    glm::vec3 extends;
    glm::mat3 rotate;       // 旋转矩阵

    bool intersactWith(const Obb& other);

    void draw(Shader* shader);
    void drawLine(Shader* shader);

    material_t material;

private:
    void generate();
    void bind();

    unsigned int VAO, VBO, EBO;
    unsigned int VAOline, EBOline;
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> line_indices;
};

#endif