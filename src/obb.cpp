#include <obb.h>
#include <object.hpp>
#include <defs.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <set>
#include <algorithm>

#include <iostream>

#define INF 1e6f

glm::mat3 CovMatrix(const std::vector<glm::vec3> &vertices);
void JacobiSolver(glm::mat3 matrix, glm::vec3 &eValues, glm::mat3 &eVectors);
void SchmidtOrthogonalization(glm::vec3 &v0, glm::vec3 &v1, glm::vec3 &v2);

// obbgen使用样例
//    Scene* scene = Scene::LoadObj("resource/assets/scene2/scene.obj");
//    std::vector<Mesh>& meshes = scene->getMesh();
//    std::vector<Obb> obbs;
//    for(int i = 0; i < meshes.size(); i++) {
//        std::vector<glm::vec3> vertices;
//        for(int j = 0; j < meshes[i].getIndex().size(); j++) {
//            index_t idx = meshes[i].getIndex()[j];
//            float x = scene->getAttrib().vertices[(idx.vid - 1) * 3];
//            float y = scene->getAttrib().vertices[(idx.vid - 1) * 3 + 1];
//            float z = scene->getAttrib().vertices[(idx.vid - 1) * 3 + 2];
//            vertices.push_back(glm::vec3(x, y, z));
//        }
//        Obb obb = Obb::obbgen(vertices);
//        obb.init();
//        obb.material.albedo = glm::vec3(1.0f, 0.0f, 0.0f);
//        obbs.emplace_back(obb);
//    }

struct foo
{
    foo() : x(0.0f), y(0.0f), z(0.0f) {}
    foo(const foo &other) : x(other.x), y(other.y), z(other.z) {}
    foo(const glm::vec3 &vec)
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }
    float x, y, z;
    bool operator==(const foo &other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }
    bool operator<(const foo &other) const
    {
        if (x == other.x)
        {
            if (y == other.y)
            {
                return z < other.z;
            }
            return y < other.y;
        }
        return x < other.x;
    }
};

Obb *Obb::obbgen(const std::vector<glm::vec3> &vertices, bool ground_parallel)
{
    // 对顶点去重
    // 由于glm::vec3没有定义<，因此先转成foo
    std::vector<foo> vec;
    for (int i = 0; i < vertices.size(); i++)
    {
        vec.emplace_back(vertices[i]);
    }
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
    std::vector<glm::vec3> vertices_unique;
    for (auto &f : vec)
    {
        vertices_unique.emplace_back(f.x, f.y, f.z);
    }

    glm::mat3 covMat = CovMatrix(vertices_unique);

    glm::vec3 eValues;
    glm::mat3 eVectors;
    // OBB的三条轴为eVectors
    JacobiSolver(covMat, eValues, eVectors);

    for (int i = 0; i < 3; i++)
    {
        if (eValues[i] == 0 || i == 2)
        {
            SchmidtOrthogonalization(eVectors[(i + 1) % 3], eVectors[(i + 2) % 3], eVectors[i]);
            break;
        }
    }

    if (ground_parallel)
    {
        // 将第二个基变为(0, 1, 0)，强制OBB与地面平行
        eVectors[1] = glm::vec3(0.0f, 1.0f, 0.0f);
        eVectors[2] = glm::normalize(glm::cross(eVectors[0], eVectors[1]));
        eVectors[0] = glm::normalize(glm::cross(eVectors[1], eVectors[2]));
    }

    glm::vec3 minExtents(INF, INF, INF);
    glm::vec3 maxExtents(-INF, -INF, -INF);

    for (const glm::vec3 &v : vertices_unique)
    {
        minExtents[0] = std::min(minExtents[0], glm::dot(v, eVectors[0]));
        minExtents[1] = std::min(minExtents[1], glm::dot(v, eVectors[1]));
        minExtents[2] = std::min(minExtents[2], glm::dot(v, eVectors[2]));

        maxExtents[0] = std::max(maxExtents[0], glm::dot(v, eVectors[0]));
        maxExtents[1] = std::max(maxExtents[1], glm::dot(v, eVectors[1]));
        maxExtents[2] = std::max(maxExtents[2], glm::dot(v, eVectors[2]));
    }

    glm::vec3 halfExtent = (maxExtents - minExtents) / 2.0f;
    glm::vec3 offset = halfExtent + minExtents;
    // Obb的中心位置: 在eVectors基下坐标为offset, 再乘以过渡矩阵
    glm::vec3 center = offset.x * eVectors[0] + offset.y * eVectors[1] + offset.z * eVectors[2];

    return new Obb(center, halfExtent, glm::transpose(eVectors));
}

Obb::Obb(const glm::vec3 &center, const glm::vec3 &extends, const glm::mat3 &rotate)
    : center(center), extends(extends), rotate(rotate)
{
}

void Obb::init()
{
    generate();
    bind();
}

void Obb::draw(Shader *shader)
{
#if OBB_SHOW
    shader->use();
    glm::mat4 model(1.0f);
    model = glm::translate(model, center);
    model = model * glm::mat4(rotate);
    model = glm::scale(model, extends);
    model = object->getGmodelObb() * model;
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(model));

    material.configShader(shader);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
#endif
}

void Obb::drawLine(Shader *shader)
{
    if (drawFlag-- <= 0)
    {
        return;
    }
#if OBB_SHOW
    shader->use();
    glm::mat4 model(1.0f);
    model = glm::translate(model, center);
    model = model * glm::mat4(rotate);
    model = glm::scale(model, extends);
    model = object->getGmodelObb() * model;
    shader->setmat4fv("model", GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAOline);
    glDrawElements(GL_LINES, line_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
#endif
}

/*
    0: 不相交
    1: 相交
    2: on
    3: under
*/
int Obb::intersectWith(Obb &other)
{

    glm::vec3 translateMat = object->getGmodelObb() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec3 otherTranslateMat = other.object->getGmodelObb() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat3 rotateMat = this->object->getGmodelObb();
    glm::mat3 otherRotateMat = other.object->getGmodelObb();

    glm::vec3 center1 = (otherRotateMat * other.center) + otherTranslateMat;
    glm::vec3 center2 = (rotateMat * this->center) + translateMat;
    glm::vec3 v = center1 - center2;

    // A旋转后的轴
    glm::vec3 VAx = rotateMat * this->rotate * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 VAy = rotateMat * this->rotate * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 VAz = rotateMat * this->rotate * glm::vec3(0.0f, 0.0f, 1.0f);

    std::vector<glm::vec3> VA = {VAx, VAy, VAz};

    // B旋转后的轴
    glm::vec3 VBx = otherRotateMat * other.rotate * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 VBy = otherRotateMat * other.rotate * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 VBz = otherRotateMat * other.rotate * glm::vec3(0.0f, 0.0f, 1.0f);

    std::vector<glm::vec3> VB = {VBx, VBy, VBz};

    glm::vec3 T(glm::dot(v, VAx), glm::dot(v, VAy), glm::dot(v, VAz));

    glm::vec3 R[3], FR[3];
    float ra, rb, t;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            R[i][j] = glm::dot(VA[i], VB[j]);
            FR[i][j] = 1e-6f + abs(R[i][j]);
        }
    }

    // split axis与A的轴平行
    for (int i = 0; i < 3; i++)
    {
        ra = this->extends[i];
        rb = other.extends[0] * FR[i][0] + other.extends[1] * FR[i][1] + other.extends[2] * FR[i][2];
        t = abs(T[i]);
        if (t > ra + rb)
            return false;
    }

    // split axis与B的轴平行
    for (int j = 0; j < 3; j++)
    {
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

    other.drawFlag = 200;
    this->drawFlag = 200;
    // return true;

    // std::cout<<"extend: "<<this->extends.x<<" "<<this->extends.y<<" "<<this->extends.z<<std::endl;
    // std::cout<<"center: "<<this->center.x<<" "<<this->center.y<<" "<<this->center.z<<std::endl;
    // std::cout<<"other extend: "<<other.extends.x<<" "<<other.extends.y<<" "<<other.extends.z<<std::endl;
    // std::cout<<"other center: "<<other.center.x<<" "<<other.center.y<<" "<<other.center.z<<std::endl;

    // std::cout<<"intersect"<<std::endl;

    // std::cout << "this->center.y - this->extends.y" << this->center.y - this->extends.y << "other.center.y + other.extends.y" << other.center.y + other.extends.y - EPS << std::endl;
    // std::cout << "this->center.y + this->extends.y" << this->center.y + this->extends.y << "other.center.y - other.extends.y" << other.center.y - other.extends.y << std::endl << std::endl;
    if((this->center.y + translateMat.y - this->extends.y) > (other.center.y + otherTranslateMat.y + other.extends.y) - EPS) {
        return INTERSECT_ON;
    }
    if((this->center.y + translateMat.y + this->extends.y) < (other.center.y + otherTranslateMat.y - other.extends.y) + EPS) {
        return INTERSECT_UNDER;
    }

    return INTERSECT_SOMETHING;

}

void Obb::generate()
{
    for (int i = -1; i <= 1; i += 2)
    {
        for (int j = -1; j <= 1; j += 2)
        {
            for (int k = -1; k <= 1; k += 2)
            {
                vertices.push_back(glm::vec3(i, j, k));
            }
        }
    }

    indices = std::vector<unsigned>({
        0,
        3,
        1,
        0,
        2,
        3,
        1,
        5,
        4,
        1,
        4,
        0,
        1,
        7,
        5,
        1,
        3,
        7,
        3,
        2,
        6,
        7,
        3,
        6,
        5,
        7,
        4,
        4,
        7,
        6,
        2,
        0,
        6,
        0,
        4,
        6,
    });

    line_indices = std::vector<unsigned>({
        0,
        2,
        0,
        1,
        1,
        3,
        2,
        3,
        0,
        4,
        2,
        6,
        3,
        7,
        1,
        5,
        4,
        5,
        5,
        7,
        6,
        7,
        4,
        6,
    });
}

void Obb::bind()
{
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // line
    glGenBuffers(1, &EBOline);
    glGenVertexArrays(1, &VAOline);

    glBindVertexArray(VAOline);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOline);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * line_indices.size(), line_indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// 计算协方差矩阵
glm::mat3 CovMatrix(const std::vector<glm::vec3> &vertices)
{
    //    for(int i = 0; i < vertices.size(); i++) {
    //        for(int j = 0; j < 3; j++) {
    //            std::cout << vertices[i][j] << " ";
    //        }
    //        std::cout << std::endl;
    //    }
    glm::mat3 cov(0.0f);
    std::vector<glm::vec3> vcopy(vertices);

    // compute average x, y, z
    glm::vec3 avg(0.0);
    for (int i = 0; i < vcopy.size(); i++)
    {
        avg += vcopy[i];
    }
    avg /= (float)vcopy.size();

    for (int i = 0; i < vcopy.size(); i++)
    {
        vcopy[i] -= avg;
    }

    // compute cov
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            for (int i = 0; i < vcopy.size(); i++)
            {
                cov[row][col] += vcopy[i][row] * vcopy[i][col];
            }
        }
    }
    cov /= (float)vcopy.size();

    return cov;
}

// 求解特征向量
void JacobiSolver(glm::mat3 matrix, glm::vec3 &eValues, glm::mat3 &eVectors)
{
    //    for(int i = 0; i < 3; i++) {
    //        for(int j = 0; j < 3; j++) {
    //            std::cout << matrix[i][j] << " ";
    //        }
    //        std::cout << std::endl;
    //    }
    const float eps1 = 0.000001f;
    const float eps2 = 0.000001f;
    const float eps3 = 0.000001f;
    const float INV_SQRT_TWO = 0.707106781f;

    float p, q, spq;
    float cosa, sina; // holds cos(alpha) and sin(alpha)
    float temp;       // used for temporary storage
    float s1 = 0.0f;  // sums of squares of diagonal
    float s2;         // elements

    bool flag = true;  // determines whether to iterate again.
    int iteration = 0; // iteration counter

    glm::vec3 mik; // used for temporary storage of m[i][k]

    glm::mat3 t = glm::mat3(1.0f); // stores the product of the rotation matrices.
                                   // Its columns ultimately hold the eigenvectors

    do
    {
        iteration++;

        for (int i = 0; i < 2; ++i)
        {
            for (int j = i + 1; j < 3; ++j)
            {
                if ((fabs(matrix[j][i]) < eps1))
                {
                    matrix[j][i] = 0.0f;
                }
                else
                {
                    q = fabs(matrix[i][i] - matrix[j][j]);

                    if (q > eps2)
                    {
                        p = 2.0f * matrix[j][i] * q / (matrix[i][i] - matrix[j][j]);
                        spq = sqrt(p * p + q * q);
                        cosa = sqrt((1.0f + q / spq) / 2.0f);
                        sina = p / (2.0f * cosa * spq);
                    }
                    else
                    {
                        sina = cosa = INV_SQRT_TWO;
                    }

                    for (int k = 0; k < 3; ++k)
                    {
                        temp = t[k][i];
                        t[k][i] = temp * cosa + t[k][j] * sina;
                        t[k][j] = temp * sina - t[k][j] * cosa;
                    }

                    for (int k = i; k < 3; ++k)
                    {
                        if (k > j)
                        {
                            temp = matrix[k][i];
                            matrix[k][i] = cosa * temp + sina * matrix[k][j];
                            matrix[k][j] = sina * temp - cosa * matrix[k][j];
                        }
                        else
                        {
                            mik[k] = matrix[k][i];
                            matrix[k][i] = cosa * mik[k] + sina * matrix[j][k];

                            if (k == j)
                            {
                                matrix[k][j] = sina * mik[k] - cosa * matrix[k][j];
                            }
                        }
                    }

                    mik[j] = sina * mik[i] - cosa * mik[j];

                    for (int k = 0; k <= j; ++k)
                    {
                        if (k <= i)
                        {
                            temp = matrix[i][k];
                            matrix[i][k] = cosa * temp + sina * matrix[j][k];
                            matrix[j][k] = sina * temp - cosa * matrix[j][k];
                        }
                        else
                        {
                            matrix[j][k] = sina * mik[k] - cosa * matrix[j][k];
                        }
                    }
                }
            }
        }

        s2 = 0.0f;

        for (int i = 0; i < 3; ++i)
        {
            eValues[i] = matrix[i][i];
            s2 += eValues[i] * eValues[i];
        }

        if (fabs(s2) < static_cast<float>(1.e-5) || fabs(1 - s1 / s2) < eps3)
        {
            flag = false;
        }
        else
        {
            s1 = s2;
        }
    } while (flag);

    eVectors[0] = t[0];
    eVectors[1] = t[1];
    eVectors[2] = t[2];

    // preserve righthanded-ness:
    if (glm::dot(glm::cross(eVectors[0], eVectors[1]), eVectors[2]) < 0.0f)
    {
        eVectors[2] = -eVectors[2];
    }
}

// 施密特正交化
void SchmidtOrthogonalization(glm::vec3 &v0, glm::vec3 &v1, glm::vec3 &v2)
{
    v0 = glm::normalize(v0);
    v1 -= glm::dot(v0, v1) * v0;
    v1 = glm::normalize(v1);
    v2 = glm::cross(v0, v1);
}

// 创建一个能包围obbs中的所有OBB的OBB
Obb* Obb::Union(const std::vector<Obb*>& obbs) {
    std::vector<glm::vec3> vertices;
    for(Obb* obb : obbs) {
        glm::vec3 p0 = obb->center + obb->extends * obb->rotate * glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 p1 = obb->center + obb->extends * obb->rotate * glm::vec3(1.0f, 1.0f, -1.0f);
        glm::vec3 p2 = obb->center + obb->extends * obb->rotate * glm::vec3(1.0f, -1.0f, 1.0f);
        glm::vec3 p3 = obb->center + obb->extends * obb->rotate * glm::vec3(1.0f, -1.0f, -1.0f);
        glm::vec3 p4 = obb->center + obb->extends * obb->rotate * glm::vec3(-1.0f, 1.0f, 1.0f);
        glm::vec3 p5 = obb->center + obb->extends * obb->rotate * glm::vec3(-1.0f, 1.0f, -1.0f);
        glm::vec3 p6 = obb->center + obb->extends * obb->rotate * glm::vec3(-1.0f, -1.0f, 1.0f);
        glm::vec3 p7 = obb->center + obb->extends * obb->rotate * glm::vec3(-1.0f, -1.0f, -1.0f);
        vertices.push_back(p0);
        vertices.push_back(p1);
        vertices.push_back(p2);
        vertices.push_back(p3);
        vertices.push_back(p4);
        vertices.push_back(p5);
        vertices.push_back(p6);
        vertices.push_back(p7);
    }
    return obbgen(vertices);
}

std::vector<glm::vec3> Obb::getBoxPoint() const {
    std::vector<glm::vec3> gv;

    glm::mat4 model(1.0f);
    model = glm::translate(model, center);
    model = model * glm::mat4(rotate);
    model = glm::scale(model, extends);
    model = object->getGmodelObb() * model;
    for(glm::vec3 vert : vertices) {
        glm::vec3 v = glm::vec3(model * glm::vec4(vert, 1.0f));
        gv.push_back(v);
    }
    return gv;
}