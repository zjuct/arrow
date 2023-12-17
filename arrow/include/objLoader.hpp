#ifndef _ASSET_HPP_
#define _ASSET_HPP_

#include "glm/gtc/type_ptr.hpp"
#include "texturemgr.hpp"
#include "shape.hpp"
#include <iostream>
#include <string>
#include <vector>

struct attrib_t {
    std::vector<float> vertices;        // v
    std::vector<float> normals;         // vn
    std::vector<float> texcoords;       // vt
};

// f v/vt/vn
struct index_t {
    int vid;        // vertex
    int tcid;       // texcoord
    int nid;        // normal
};

enum tuple_type {
    TUPLE_NONE, TUPLE_V, TUPLE_V_VN, TUPLE_V_VT, TUPLE_V_VT_VN,
};

class Scene;

class Mesh: public Shape {
public:
    Mesh(const std::string& _name, const std::vector<index_t>& _indices,
        unsigned int _vertices_per_face, tuple_type _type,
        int _material, attrib_t* _attrib, TextureMgr* _texmgr)
        : name(_name), indices(_indices), vertices_per_face(_vertices_per_face),
          type(_type), material(_material), attrib(_attrib), texmgr(_texmgr) {
        nrfaces = indices.size() / vertices_per_face;
        setupMesh();
    }

    void draw(Shader* shader);
    void setScene(Scene* scene) {
        this->scene = scene;
    }
private:
    void setupMesh();

    std::string name;
    std::vector<index_t> indices;       // index into Scene::attrib
    unsigned int vertices_per_face;
    unsigned int nrfaces;
    int material;       // index into Scene::materials
    unsigned int VAO, VBO;
    attrib_t* attrib;
    TextureMgr* texmgr;
    tuple_type type;
    Scene* scene;
};


/* LoadObj的返回值 */
class Scene {
public:
    static Scene* LoadObj(std::string file, TextureMgr* texmgr);
    std::vector<Mesh>& getMesh() {
        return meshes;
    }
    attrib_t& getAttrib() {
        return attrib;
    }
    std::vector<material_t>& getMaterials() {
        return materials;
    }

private:
    Scene(attrib_t _attrib, std::vector<Mesh> _meshes, std::vector<material_t> _materials)
        : attrib(_attrib), meshes(_meshes), materials(_materials) {

    }

    attrib_t attrib;
    std::vector<Mesh> meshes;
    std::vector<material_t> materials;
};


#endif