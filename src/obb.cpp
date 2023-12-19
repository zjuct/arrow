#include <obb.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

Obb::Obb(const glm::vec3& center, const glm::vec3& extends, const glm::mat3& rotate)
    : center(center), extends(extends), rotate(rotate) {
    
}

void Obb::init() {
    generate();
    bind();
}

void Obb::draw(Shader* shader) {
    shader->use();
    glm::mat4 model(1.0f);
    model = glm::translate(model, center);
    model = model * glm::mat4(rotate);
    model = glm::scale(model, extends);
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(model));

    material.configShader(shader);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

bool Obb::intersactWith(const Obb& other) {
    glm::vec3 v = other.center - this->center;

    // A旋转后的轴
    glm::vec3 VAx = this->rotate * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 VAy = this->rotate * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 VAz = this->rotate * glm::vec3(0.0f, 0.0f, 1.0f);

    std::vector<glm::vec3> VA = { VAx, VAy, VAz };

    // B旋转后的轴
    glm::vec3 VBx = other.rotate * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 VBy = other.rotate * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 VBz = other.rotate * glm::vec3(0.0f, 0.0f, 1.0f);

    std::vector<glm::vec3> VB = { VBx, VBy, VBz };

    glm::vec3 T(glm::dot(v, VAx), glm::dot(v, VAy), glm::dot(v, VAz));

    glm::vec3 R[3], FR[3];
    float ra, rb, t;
    
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            R[i][j] = glm::dot(VA[i], VB[j]);
            FR[i][j] = 1e-6f + abs(R[i][j]);
        }
    }

    // split axis与A的轴平行
    for(int i = 0; i < 3; i++) {
        ra = this->extends[i];
        rb = other.extends[0] * FR[i][0] + other.extends[1] * FR[i][1] + other.extends[2] * FR[i][2];
        t = abs(T[i]);
        if(t > ra + rb)
            return false;
    }

    // split axis与B的轴平行
    for(int j = 0; j < 3; j++) {
        ra = this->extends[0] * FR[0][j] + this->extends[1] * FR[1][j] + this->extends[2] * FR[2][j];
        rb = other.extends[j];
        t = abs(T[0] * R[0][j] + T[1] * R[1][j] + T[2] * R[2][j]);
        if (t > ra + rb)
            return false;
    }

    // Ax x Bx
    ra = this->extends[1] * FR[2][0] + this->extends[2] * FR[1][0];
    rb = other.extends[1] * FR[0][2] + other.extends[2] * FR[0][1];
    t = abs(T[2] * R[1][0] - T[1] * R[2][0]);
    if (t > ra + rb)
        return false;

    // Ax x By
    ra = this->extends[1] * FR[2][1] + this->extends[2] * FR[1][1];
    rb = other.extends[0] * FR[0][2] + other.extends[2] * FR[0][0];
    t = abs(T[2] * R[1][1] - T[1] * R[2][1]);
    if (t > ra + rb)
        return false;

    // Ax x Bz
    ra = this->extends[1] * FR[2][2] + this->extends[2] * FR[1][2];
    rb = other.extends[0] * FR[0][1] + other.extends[1] * FR[0][0];
    t = abs(T[2] * R[1][2] - T[1] * R[2][2]);
    if (t > ra + rb)
        return false;

    // Ay x Bx
    ra = this->extends[0] * FR[2][0] + this->extends[2] * FR[0][0];
    rb = other.extends[1] * FR[1][2] + other.extends[2] * FR[1][1];
    t = abs(T[0] * R[2][0] - T[2] * R[0][0]);
    if (t > ra + rb)
        return false;

    // Ay x By
    ra = this->extends[0] * FR[2][1] + this->extends[2] * FR[0][1];
    rb = other.extends[0] * FR[1][2] + other.extends[2] * FR[1][0];
    t = abs(T[0] * R[2][1] - T[2] * R[0][1]);
    if (t > ra + rb)
        return false;

    // Ay x Bz
    ra = this->extends[0] * FR[2][2] + this->extends[2] * FR[0][2];
    rb = other.extends[0] * FR[1][1] + other.extends[1] * FR[1][0];
    t = abs(T[0] * R[2][2] - T[2] * R[0][2]);
    if (t > ra + rb)
        return false;

    // Az x Bx
    ra = this->extends[0] * FR[1][0] + this->extends[1] * FR[0][0];
    rb = other.extends[1] * FR[2][2] + other.extends[2] * FR[2][1];
    t = abs(T[1] * R[0][0] - T[0] * R[1][0]);
    if (t > ra + rb)
        return false;

    // Az x By
    ra = this->extends[0] * FR[1][1] + this->extends[1] * FR[0][1];
    rb = other.extends[0] * FR[2][2] + other.extends[2] * FR[2][0];
    t = abs(T[1] * R[0][1] - T[0] * R[1][1]);
    if (t > ra + rb)
        return false;

    // Az x Bz
    ra = this->extends[0] * FR[1][2] + this->extends[1] * FR[0][2];
    rb = other.extends[0] * FR[2][1] + other.extends[1] * FR[2][0];
    t = abs(T[1] * R[0][2] - T[0] * R[1][2]);
    if (t > ra + rb)
        return false;

    return true;
}

void Obb::generate() {
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

void Obb::bind() {
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