#ifndef _OBB_H
#define _OBB_H

#include <shader.h>
#include <material.h>
#include <glm/glm.hpp>

#include <vector>

class Object;

enum intersectType {
    INTERSECT_NONE = 0,
    INTERSECT_SOMETHING,
    INTERSECT_ON,
    INTERSECT_UNDER,
};

class Obb {
public:
    static Obb* obbgen(const std::vector<glm::vec3>& vertices, bool ground_parallel = true);
    static Obb* Union(const std::vector<Obb*>& obbs);

    Obb(const glm::vec3& center = glm::vec3(0.0f), const glm::vec3& extends = glm::vec3(1.0f),
        const glm::mat3& rotate = glm::mat3(1.0f));

    Obb(const Obb& o) {
        center = o.center;
        extends = o.extends;
        rotate = o.rotate;

        material = o.material;
        VAO = o.VAO;
        VBO = o.VBO;
        EBO = o.EBO;
        VAOline = o.VAOline;
        EBOline = o.EBOline;
        vertices = o.vertices;
        indices = o.indices;
        line_indices = o.line_indices;
        object = o.object;
    }
    
    Obb& operator=(const Obb& o) {
        if(this == &o) {
            return *this;
        }
        center = o.center;
        extends = o.extends;
        rotate = o.rotate;

        material = o.material;
        VAO = o.VAO;
        VBO = o.VBO;
        EBO = o.EBO;
        VAOline = o.VAOline;
        EBOline = o.EBOline;
        vertices = o.vertices;
        indices = o.indices;
        line_indices = o.line_indices;
        object = o.object;
        return *this;
    }

    void init();

    int maxExtent() const {
        if(extends[0] > extends[1]) {
            if(extends[0] > extends[2])
                return 0; 
            else 
                return 2;
        } else {
            if(extends[1] > extends[2])
                return 1;
            else
                return 2;
        }
    }

    std::vector<glm::vec3> getBoxPoint() const;

    glm::vec3 center;
    glm::vec3 extends;
    glm::mat3 rotate;       // 旋转矩阵

    Object* object;     // OBB所属的object

    int intersectWith(Obb& other);

    void draw(Shader* shader);
    void drawLine(Shader* shader);

    material_t material;

    int drawFlag = 0;

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