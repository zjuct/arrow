#include <shape.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 fNormal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
    glm::vec3 e12 = v2 - v1;
    glm::vec3 e13 = v3 - v1;

    return glm::cross(e12, e13);
}

// 面法向量面积加权计算顶点法向量，只支持三角形面
void calcVNormal(std::vector<vertex_t>& v, const std::vector<unsigned int>& indices) {
    unsigned int vi1, vi2, vi3;
    glm::vec3 v1, v2, v3, n;
    for(int i = 0; i < indices.size(); i += 3) {
        vi1 = indices[i];
        vi2 = indices[i + 1];
        vi3 = indices[i + 2];
        
        v1 = v[vi1].vertices;
        v2 = v[vi2].vertices;
        v3 = v[vi3].vertices;

        // 未normalize的面法向量已经是面积的加权
        n = fNormal(v1, v2, v3);
        
        v[vi1].normals += n;
        v[vi2].normals += n;
        v[vi3].normals += n;
    }

    for(int i = 0; i < v.size(); i++) {
        v[i].normals = glm::normalize(v[i].normals);
    }
}