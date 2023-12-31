#include <objLoader.hpp>
#include <texturemgr.hpp>
#include <material.h>
#include <defs.h>

#include <iostream>
#include <map>
#include <sstream>
#include <cstring>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t'))

tuple_type gettupletype(std::vector<std::string>& tokens) {
    switch(tokens.size()) {
        case 1:
            return TUPLE_V;
        case 2:
            return TUPLE_V_VT;
        case 3:
            if(tokens[1] == "") 
                return TUPLE_V_VN;
            else
                return TUPLE_V_VT_VN;
        default:
            return TUPLE_NONE;
    }
}

std::vector<std::string> strspl(const std::string& str, char delim) {
    std::vector<std::string> result;
    std::string current;

    for(char c: str) {
        if(c == delim) {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}

/*
.obj文件支持Mesh中的各个face有不同的边数和材质 
这里为了渲染方便，每个Mesh仅有一种材质
TODO: 增加对Mesh内包含不同形状face的支持
*/
bool commitMesh(const std::string& name, const std::vector<index_t>& indices,
    unsigned int vertex_per_face, tuple_type type, int material,
    std::vector<Mesh>& meshes, attrib_t* attrib, Scene* scene, bool triangulate = true) {
    if(indices.empty())
        return true;

    if(vertex_per_face < 3) {
        std::cerr << "[ERROR] Face must have 3+ vertices" << std::endl;
        return false;
    }

    if(triangulate && vertex_per_face != 3) {
        std::vector<index_t> tri_indices;
        std::vector<float>& v = attrib->vertices;

        for(int i = 0; i < indices.size(); i += vertex_per_face) {
            index_t i0 = indices[i];
            index_t i1 = indices[i + 1];
            index_t i2 = indices[i + 2];
            index_t i3 = indices[i + 3];

            int vi0 = i0.vid - 1;
            int vi1 = i1.vid - 1;
            int vi2 = i2.vid - 1;
            int vi3 = i3.vid - 1;

            if (((3 * vi0 + 2) >= v.size()) || ((3 * vi1 + 2) >= v.size()) ||
                ((3 * vi2 + 2) >= v.size()) || ((3 * vi3 + 2) >= v.size())) {
                std::cerr << "[ERROR] Vertex indices " << vi0 << " " << vi1 << " " << vi2 << " " << vi3 << " out of bound." << std::endl;
                return false;
            }

            float v0x = v[vi0 * 3 + 0];
            float v0y = v[vi0 * 3 + 1];
            float v0z = v[vi0 * 3 + 2];
            float v1x = v[vi1 * 3 + 0];
            float v1y = v[vi1 * 3 + 1];
            float v1z = v[vi1 * 3 + 2];
            float v2x = v[vi2 * 3 + 0];
            float v2y = v[vi2 * 3 + 1];
            float v2z = v[vi2 * 3 + 2];
            float v3x = v[vi3 * 3 + 0];
            float v3y = v[vi3 * 3 + 1];
            float v3z = v[vi3 * 3 + 2];

            // Two choices to split the quad into two triangles
            // Choose the shortest edge
            // +---+
            // |\  |
            // | \ |
            // |  \|
            // +---+
            //
            // +---+
            // |  /|
            // | / |
            // |/  |
            // +---+

            float e02x = v2x - v0x;
            float e02y = v2y - v0y;
            float e02z = v2z - v0z;
            float e13x = v3x - v1x;
            float e13y = v3y - v1y;
            float e13z = v3z - v1z;

            float sqr02 = e02x * e02x + e02y * e02y + e02z * e02z;
            float sqr13 = e13x * e13x + e13y * e13y + e13z * e13z;

            if(sqr02 < sqr13) {
                // [0, 1, 2,], [0, 2, 3]
                tri_indices.push_back(i0);
                tri_indices.push_back(i1);
                tri_indices.push_back(i2);

                tri_indices.push_back(i0);
                tri_indices.push_back(i2);
                tri_indices.push_back(i3);
            } else {
                // [0, 1, 3], [1, 2, 3]
                tri_indices.push_back(i0);
                tri_indices.push_back(i1);
                tri_indices.push_back(i3);

                tri_indices.push_back(i1);
                tri_indices.push_back(i2);
                tri_indices.push_back(i3);
            }
        }
        
        vertex_per_face = 3;
        meshes.push_back(Mesh(name, tri_indices, vertex_per_face, type, material, attrib, scene));
        meshes.back().setupMesh();
    } else {
        meshes.push_back(Mesh(name, indices, vertex_per_face, type, material, attrib, scene));
        meshes.back().setupMesh();
    }
    return true;
}

bool loadmtl(const std::string& file, const std::string& basedir, std::vector<material_t>& materials, std::map<std::string, int>& material_map) {
    std::string path = basedir + file;
    std::ifstream ifs(path);
    if(!ifs)
        return false;

    material_t m;

    std::stringstream ss;
    std::string line, op;
    int lineno = -1;
    while(!ifs.eof()) {
        // 清空stingstream要用ss.str("")，而非ss.clear()
        // 重置stringstream要用ss.str()和ss.clear()
        ss.clear();
        ss.str("");
        std::getline(ifs, line);
        lineno++;

        // Trim newline \r\n or \n
        if(line.size() > 0 && line[line.size() - 1] == '\n')
            line.erase(line.size() - 1);
        if(line.size() > 0 && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        // Skip empty line
        if(line.empty()) 
            continue;

        const char* token = line.c_str();
        // Skip loading space
        token += strspn(token, " \t");

        // Skip empty line and comment line
        if(token[0] == '\0' || token[0] == '#')
            continue;

        ss.str(token);
        ss >> op;

        if(op == "newmtl") {
            if(!m.name.empty()) {
                material_map.insert(std::pair<std::string, int>(m.name, materials.size()));
                materials.push_back(m);
            }

            m = material_t();

            std::string name;
            ss >> name;
            if(name.empty()) {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Empty material name." << std::endl;
                return false;
            }
            m.name = name;
            continue;
        }

        if(op == "Ka") {
            if(ss.eof()) 
                continue;
            float r, g, b;
            ss >> r >> g >> b;
            m.ambient[0] = r;
            m.ambient[1] = g;
            m.ambient[2] = b;
            continue;
        }

        if(op == "Kd") {
            if(ss.eof()) 
                continue;
            float r, g, b;
            ss >> r >> g >> b;
            m.diffuse[0] = r;
            m.diffuse[1] = g;
            m.diffuse[2] = b;
            continue;
        }

        if(op == "Ks") {
            if(ss.eof()) 
                continue;
            float r, g, b;
            ss >> r >> g >> b;
            m.specular[0] = r;
            m.specular[1] = g;
            m.specular[2] = b;
            continue;
        }

        if(op == "illum") {
            if(ss.eof()) 
                continue;
            ss >> m.illum;
            continue;
        }

        if(op == "Ns") {
            if(ss.eof()) 
                continue;
            ss >> m.shininess;
            continue;
        }

        if(op == "map_Ka") {
            if(ss.eof()) 
                continue;
            std::string texfile;
            ss >> texfile;
            if(!texfile.empty()) {
                texfile = basedir + texfile;
                if(!TextureMgr::getInstance()->load(texfile, TEX_2D)) {
                    std::cerr << "[ERROR] " << file << ":" << lineno << " Failed to load texture." << std::endl;
                    return false;
                }
                m.ambient_texname = texfile;
            } else {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Empty map_Ka name." << std::endl;
                return false;
            }
            continue;
        }

        if(op == "map_Kd") {
            if(ss.eof()) 
                continue;
            std::string texfile;
            ss >> texfile;
            if(!texfile.empty()) {
                texfile = basedir + texfile;
                if(!TextureMgr::getInstance()->load(texfile, TEX_2D)) {
                    std::cerr << "[ERROR] " << file << ":" << lineno << " Failed to load texture." << std::endl;
                    return false;
                }
                m.diffuse_texname = texfile;
            } else {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Empty map_Ka name." << std::endl;
                return false;
            }
            continue;
        }

        if(op == "map_Ks") {
            if(ss.eof()) 
                continue;
            std::string texfile;
            ss >> texfile;
            if(!texfile.empty()) {
                texfile = basedir + texfile;
                if(!TextureMgr::getInstance()->load(texfile, TEX_2D)) {
                    std::cerr << "[ERROR] " << file << ":" << lineno << " Failed to load texture." << std::endl;
                    return false;
                }
                m.specular_texname = texfile;
            } else {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Empty map_Ka name." << std::endl;
                return false;
            }
            continue;
        }

        if(op == "map_Bump" || op == "bump") {
            if(ss.eof()) 
                continue;
            std::string texfile;
            ss >> texfile;
            if(!texfile.empty()) {
                texfile = basedir + texfile;
                if(!TextureMgr::getInstance()->load(texfile, TEX_2D)) {
                    std::cerr << "[ERROR] " << file << ":" << lineno << " Failed to load texture." << std::endl;
                    return false;
                }
                m.bump_texname = texfile;
            } else {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Empty map_Ka name." << std::endl;
                return false;
            }
            continue;
        }
    }

    // commit last material
    if(!m.name.empty()) {
        material_map.insert(std::pair<std::string, int>(m.name, materials.size()));
        materials.push_back(m);
    }
    return true;
}

Scene* Scene::LoadObj(std::string file, bool PRT, std::string SHL, std::string SHLT) {
    Scene* s = new Scene();
    s->PRT = false;

#if PRT_ENABLE
    if(PRT) {
        s->PRT = PRT;
        std::ifstream ifs(SHL);
        if(!ifs.is_open()) {
            std::cerr << "[ERROR] Can not open file " << SHL << std::endl;
        }
        while(!ifs.eof()) {
            float r, g, b;
            ifs >> r >> g >> b;
            s->SHL[0].push_back(r);
            s->SHL[1].push_back(g);
            s->SHL[2].push_back(b);
        }
        
        s->SHLT = std::ifstream(SHLT);
        if(!s->SHLT) {
            std::cerr << "[ERROR] Can not open file " << SHLT << std::endl;
        }
    }
#endif

    attrib_t& attrib = s->attrib;
    std::vector<Mesh>& meshes = s->meshes;
    std::vector<material_t>& materials = s->materials;

    std::ifstream ifs(file);
    if(!ifs) {
        std::cerr << "[INFO] Cannot open file " << file << "." << std::endl;
        return nullptr;
    }

    std::vector<index_t> indices;
    int vertices_per_face = -1;
    tuple_type type = TUPLE_NONE;
    std::string name;

    std::map<std::string, int> material_map;        // name --> index into materials
    int material = -1;      // current material

    std::string line = "", op;
    std::stringstream ss;
    int lineno = -1;
    while(!ifs.eof()) {
        ss.clear();
        std::getline(ifs, line);
        lineno++;

        // Trim newline \r\n or \n
        if(line.size() > 0 && line[line.size() - 1] == '\n')
            line.erase(line.size() - 1);
        if(line.size() > 0 && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        // Skip empty line
        if(line.empty()) 
            continue;

        const char* token = line.c_str();
        // Skip loading space
        token += strspn(token, " \t");

        // Skip empty line and comment line
        if(token[0] == '\0' || token[0] == '#')
            continue;

        ss.str(token);
        ss >> op;

        // 'v': vertex
        if(op == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            attrib.vertices.push_back(x);
            attrib.vertices.push_back(y);
            attrib.vertices.push_back(z);
            continue;
        }

        // 'vn': normal
        if(op == "vn") {
            float x, y, z;
            ss >> x >> y >> z;
            attrib.normals.push_back(x);
            attrib.normals.push_back(y);
            attrib.normals.push_back(z);
            continue;
        }

        // 'vt': texcoord
        if(op == "vt") {
            float u, v;
            ss >> u >> v;
            attrib.texcoords.push_back(u);
            attrib.texcoords.push_back(1.0f - v);
            continue;
        }

        // face
        if(op == "f") {
            char buf[1024];
            token += 2;
            token += strspn(token, " \t");
            strcpy(buf, token);

            char* tuple, *p = buf;
            tuple_type t = TUPLE_NONE;
            unsigned int cnt = 0;
            while((tuple = strtok(p, " \t")) != nullptr) {
                cnt++;
                p = nullptr;
                std::vector<std::string> tokens = strspl(tuple, '/');
                tuple_type tt = gettupletype(tokens);
                if(t == TUPLE_NONE) {
                    t = tt;
                } else if(tt != t) {
                    // 一个face的所有顶点必须有相同的格式
                    std::cerr << "[ERROR] " << file << ":" << lineno << " Vertices in one face should have the same format." << std::endl;
                    return nullptr;
                }

                index_t idx;
                switch(tt) {
                    case TUPLE_NONE:
                        std::cerr << "[ERROR] " << file << ":" << lineno << " Invalid vertex format in face definition." << std::endl;
                        return nullptr;
                    case TUPLE_V:
                        idx = {.vid = std::stoi(tokens[0]), .tcid = 0, .nid = 0};
                        break;
                    case TUPLE_V_VN:
                        idx = {.vid = std::stoi(tokens[0]), .tcid = 0, .nid = std::stoi(tokens[2])};
                        break;
                    case TUPLE_V_VT_VN:
                        idx = {.vid = std::stoi(tokens[0]), .tcid = std::stoi(tokens[1]), .nid = std::stoi(tokens[2])};
                        break;
                    case TUPLE_V_VT:
                        idx = {.vid = std::stoi(tokens[0]), .tcid = std::stoi(tokens[1]), .nid = 0};
                        break;
                }
                if(idx.vid < 0)
                    idx.vid += (attrib.vertices.size() / 3) + 1;
                if(idx.tcid < 0)
                    idx.tcid += (attrib.texcoords.size() / 2) + 1;
                if(idx.nid < 0)
                    idx.nid += (attrib.normals.size() / 3) + 1;

                indices.push_back(idx);
            }
            type = t;
            if(cnt < 3) {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Face should have at least 3 vertices." << std::endl;
                return nullptr;
            }
            if(vertices_per_face == -1) {
                vertices_per_face = cnt;
            } else if(vertices_per_face != cnt) {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Faces in one mesh should have the same number of vertices." << std::endl;
                return nullptr;
            }

            continue;
        }

        // use mtl
        if(op == "usemtl") {
            std::string mtlname;
            ss >> mtlname;
            int new_material = -1;
            auto it = material_map.find(mtlname);
            if(it != material_map.end()) {
                new_material = it->second;
            } else {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Material not found." << std::endl;
                return nullptr;
            }

            if(new_material != material) {
                if(!commitMesh(name, indices, vertices_per_face, type, material, meshes, &attrib, s)) {
                    std::cerr << "[ERROR] " << file << ":" << lineno << " Failed to commit mesh" << std::endl;
                }
                indices.clear();
                vertices_per_face = -1;
                material = new_material;
            }

            continue;
        }

        // load mtl
        if(op == "mtllib") {
            token += 7;
            token += strspn(token, " \t");
            char buf[1024];
            strcpy(buf, token);
            char* p = buf, *q;
            const char* basedir = strrchr(file.c_str(), '/');
            if(basedir > file.c_str() || file[0] == '/') {
                basedir++;
            }
            while((q = strtok(p, " \t")) != nullptr) {
                p = nullptr;
                if(!loadmtl(q, file.substr(0, basedir - file.c_str()), materials, material_map)) {
                    std::cerr << "[ERROR] " << file << ":" << lineno << " Failed to load mtl." << std::endl;
                }
            }
            continue;
        }

        // object name
        if(op == "o" || op == "g") {
            // commit faces to meshes
            if(!commitMesh(name, indices, vertices_per_face, type, material, meshes, &attrib, s)) {
                std::cerr << "[ERROR] " << file << ":" << lineno << " Failed to commit mesh" << std::endl;
            }
            indices.clear();
            vertices_per_face = -1;
            material = -1;

            ss >> name;

            continue;
        }
    }
    
    // commit last mesh
    if(!commitMesh(name, indices, vertices_per_face, type, material, meshes, &attrib, s)) {
        std::cerr << "[ERROR] " << file << ":" << lineno << " Failed to commit mesh" << std::endl;
    }
//    Scene* s = new Scene(attrib, meshes, materials);
    for(int i = 0; i < s->meshes.size(); i++) {
        s->meshes[i].setScene(s);
    }


    return s;
}

#define SHOrder 2

int cnt = 0;
void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

//    std::vector<float> buf;

    if(scene->PRTenable()) {
        int SHCoeffCount = 9;
        for(int i = 0; i < indices.size(); i++) {
            buf.push_back(attrib->vertices[(indices[i].vid - 1) * 3]);
            buf.push_back(attrib->vertices[(indices[i].vid - 1) * 3 + 1]);
            buf.push_back(attrib->vertices[(indices[i].vid - 1) * 3 + 2]);
            buf.push_back(attrib->normals[(indices[i].nid - 1) * 3]);
            buf.push_back(attrib->normals[(indices[i].nid - 1) * 3 + 1]);
            buf.push_back(attrib->normals[(indices[i].nid - 1) * 3 + 2]);
            buf.push_back(attrib->texcoords[(indices[i].tcid - 1) * 2]);
            buf.push_back(attrib->texcoords[(indices[i].tcid - 1) * 2 + 1]);
            
            for(int k = 0; k < 3; k++) {
                for(int j = 0; j < SHCoeffCount; j++) {
                    float shcoeff;
                    scene->SHLTstream() >> shcoeff;
                    buf.push_back(shcoeff);
                }
            }
//            std::cout << ++cnt << std::endl;

        }
        glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), &buf[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);   
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);   
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);  
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);   
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(11 * sizeof(float)));
        glEnableVertexAttribArray(4);   
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(14 * sizeof(float)));
        glEnableVertexAttribArray(5);   
//        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)0);
//        glEnableVertexAttribArray(0);   
//        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(3 * sizeof(float)));
//        glEnableVertexAttribArray(1);   
//        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(6 * sizeof(float)));
//        glEnableVertexAttribArray(2);  
//        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(8 * sizeof(float)));
//        glEnableVertexAttribArray(3);   
//        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(11 * sizeof(float)));
//        glEnableVertexAttribArray(4);   
//        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(14 * sizeof(float)));
//        glEnableVertexAttribArray(5);   
//
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(17 * sizeof(float)));
        glEnableVertexAttribArray(6);   
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(20 * sizeof(float)));
        glEnableVertexAttribArray(7);   
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(23 * sizeof(float)));
        glEnableVertexAttribArray(8);   

        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(26 * sizeof(float)));
        glEnableVertexAttribArray(9);   
        glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(29 * sizeof(float)));
        glEnableVertexAttribArray(10);   
        glVertexAttribPointer(11, 3, GL_FLOAT, GL_FALSE, (8 + 27) * sizeof(float), (void*)(32 * sizeof(float)));
        glEnableVertexAttribArray(11);   
//        glVertexAttribPointer(3, SHCoeffCount, GL_FLOAT, GL_FALSE, (8 + 3 * SHCoeffCount) * sizeof(float), (void*)(8 * sizeof(float)));
//        glEnableVertexAttribArray(3);   
//        glVertexAttribPointer(4, SHCoeffCount, GL_FLOAT, GL_FALSE, (8 + 3 * SHCoeffCount) * sizeof(float), (void*)((8 + SHCoeffCount) * sizeof(float)));
//        glEnableVertexAttribArray(4);   
//        glVertexAttribPointer(5, SHCoeffCount, GL_FLOAT, GL_FALSE, (8 + 3 * SHCoeffCount) * sizeof(float), (void*)((8 + 2 * SHCoeffCount) * sizeof(float)));
//        glEnableVertexAttribArray(5);   
    } else {
        for(int i = 0; i < indices.size(); i++) {
            // 注意.obj的下标是1开始
            buf.push_back(attrib->vertices[(indices[i].vid - 1) * 3]);
            buf.push_back(attrib->vertices[(indices[i].vid - 1) * 3 + 1]);
            buf.push_back(attrib->vertices[(indices[i].vid - 1) * 3 + 2]);
            
            if(indices[i].nid > 0) {
                buf.push_back(attrib->normals[(indices[i].nid - 1) * 3]);
                buf.push_back(attrib->normals[(indices[i].nid - 1) * 3 + 1]);
                buf.push_back(attrib->normals[(indices[i].nid - 1) * 3 + 2]);
            }

            if(indices[i].tcid > 0) {
                buf.push_back(attrib->texcoords[(indices[i].tcid - 1) * 2]);
                buf.push_back(attrib->texcoords[(indices[i].tcid - 1) * 2 + 1]);
            }
        }

        glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), &buf[0], GL_STATIC_DRAW);

        // TODO: 设置has_normal, has_tex uniform变量
        switch(type) {
            case TUPLE_V:
                glEnableVertexAttribArray(0);   
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                break;
            case TUPLE_V_VN:
                glEnableVertexAttribArray(0);   
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);   
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
                break;
            case TUPLE_V_VT_VN:
                glEnableVertexAttribArray(0);   
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);   
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(2);   
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
                break;
            case TUPLE_V_VT:
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);   
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);   
                break;
            default:
                break;
        }
    }
    glBindVertexArray(0);
}

void Mesh::draw(Shader* shader) {
    // 由于OpenGL不支持VBO用GL_POLYGON图元，因此多边形Mesh是无法绘制的
    if(vertices_per_face != 3) {
        std::cerr << "[WARNING] Polygon mesh is unable to render. Use triangle mesh instead or enable trianglize." << std::endl;;
        return;
    }
    if(material != -1) {
        material_t m = scene->getMaterials()[material];
        m.configShader(shader);
    }
    shader->use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, indices.size());
    glBindVertexArray(0);
}

Obb* Mesh::getObb() {
    std::vector<glm::vec3> vertices;
//    std::cout << attrib->vertices.size() << std::endl;
    for(index_t idx : indices) {
        int vidx = idx.vid;
//        std::cout << vidx << std::endl;
        float x = attrib->vertices[(vidx - 1) * 3];
        float y = attrib->vertices[(vidx - 1) * 3 + 1];
        float z = attrib->vertices[(vidx - 1) * 3 + 2];
        vertices.push_back(glm::vec3(x, y, z));
    }

//    for(int i = 0; i < vertices.size(); i++) {
//        for(int j = 0; j < 3; j++)
//            std::cout << vertices[i][j] << " ";
//        std::cout << std::endl;
//    }
    return Obb::obbgen(vertices);
}