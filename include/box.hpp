#ifndef _BOX_HPP_
#define _BOX_HPP_

#include "shape.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <vector>
#include "shape.hpp"

#include <obb.h>

// 不含normal, texcoords的纯色Box
// 如果需要带normal, texcoords的box，用.obj
class Box: public Shape {
public:
    Box(){}
    Box(const glm::vec3& c): color(c) {
        generate();
        bind();

//        material.skybox_texname = "assets/skybox_zjg";
    }

    void draw(Shader* shader) {
//        material.configShader(shader);
        shader->use();
//        glm::mat4 model(1.0f);
//        model = glm::scale(model, glm::vec3(10.0f, 10.0f, 10.0f));
//        shader->setmat4fv("model", GL_FALSE, glm::value_ptr(model));
//        shader->setvec3fv("color", glm::value_ptr(color));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    Obb* getObb() {
        return nullptr;
    }


private:
    void generate() {
        for(int i = -1; i <= 1; i += 2) {
            for(int j = -1; j <= 1; j += 2) {
                for(int k = -1; k <= 1; k += 2) {
                    vertices.push_back(glm::vec3(i, j, k));
                }
            }
        }

        indices = std::vector<unsigned> ({
            0, 3, 1,
            0, 2, 3,
            1, 5, 4,
            1, 4, 0,
            1, 7, 5,
            1, 3, 7,
            3, 2, 6,
            7, 3, 6,
            5, 7, 4,
            4, 7, 6,
            2, 0, 6,
            0, 4, 6,
        });
    }

    void bind() {
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glGenVertexArrays(1, &VAO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    unsigned int VBO, VAO, EBO;
    glm::vec3 color;

//    material_t material;
};


#endif