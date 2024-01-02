#include <loader.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Texture {
    // u, v \in [0, 1]
    glm::vec3 LookUp(float u, float v) {
        // 双线性差值
        assert(u >= 0.0f && u <= 1.0f);
        assert(v >= 0.0f && v <= 1.0f);
        float x = u * width, y = v * height;
        int x0 = std::min((int)x, width - 1);
        int y0 = std::min((int)y, height - 1);
        int x1 = std::min(x0 + 1, width - 1);
        int y1 = std::min(y0 + 1, height - 1);
        glm::vec3 c00(data[channel * (y0 * width + x0) + 0], data[channel * (y0 * width + x0) + 1], data[channel * (y0 * width + x0) + 2]);
        glm::vec3 c01(data[channel * (y0 * width + x1) + 0], data[channel * (y0 * width + x1) + 1], data[channel * (y0 * width + x1) + 2]);
        glm::vec3 c10(data[channel * (y1 * width + x0) + 0], data[channel * (y1 * width + x0) + 1], data[channel * (y1 * width + x0) + 2]);
        glm::vec3 c11(data[channel * (y1 * width + x1) + 0], data[channel * (y1 * width + x1) + 1], data[channel * (y1 * width + x1) + 2]);

        u = glm::clamp(x - x0, 0.0f, 1.0f);
        v = glm::clamp(y - y0, 0.0f, 1.0f);
        glm::vec3 c0 = (1.0f - v) * c00 + v * c10;
        glm::vec3 c1 = (1.0f - v) * c01 + v * c11;

        return ((1.0f - u) * c0 + u * c1) / 256.0f;
    }

    unsigned char* data;
    int width, height, channel;
};

static std::map<std::string, Texture> textures;

std::vector<float*> LoadCubemapImages(const std::string& cubemapDir, int& width, int& height, int& channel) {
    std::vector<std::string> cubemapNames{"negx.jpg", "posx.jpg", "posy.jpg",
                                        "negy.jpg", "posz.jpg", "negz.jpg"};
    std::vector<float*> images(6);
    for (int i = 0; i < 6; i++)
    {
        std::string filename = cubemapDir + "/" + cubemapNames[i];
        int w, h, c;
        float *image = stbi_loadf(filename.c_str(), &w, &h, &c, 3);
        if (!image)
        {
            std::cout << "Failed to load image: " << filename << std::endl;
            exit(-1);
        }
        if (i == 0)
        {
            width = w;
            height = h;
            channel = c;
        }
        else if (w != width || h != height || c != channel)
        {
            std::cout << "Dismatch resolution for 6 images in cubemap" << std::endl;
            exit(-1);
        }
        images[i] = image;
    }
    return images;
}

void LoadTexture(const std::string& texfile, const std::string& dir) {
    if(textures.find(texfile) != textures.end()) {
        return;
    }

    std::string path = dir + texfile;
    Texture t;
    t.data = stbi_load(path.c_str(), &t.width, &t.height, &t.channel, 0);
    if(!t.data) {
        std::cerr << "[ERROR] Failed to load " << path << std::endl;
        return;
    }
    textures.insert(std::pair<std::string, Texture>(texfile, t));
}

struct Material {
    Material(): albedo(1.0f), albedo_map("") {}
    glm::vec3 albedo;
    std::string albedo_map;
};


void LoadMtl(const char* mtlfile, const char* dir, std::map<std::string, Material>& materials) {
    std::string path = std::string(dir) + std::string(mtlfile);

    std::ifstream ifs(path);
    if(!ifs) {
        std::cerr << "[INFO] Cannot open file " << path << "." << std::endl;
        return;
    }

    Material m;
    std::string name;
    while(!ifs.eof()) {
        std::string line;
        std::getline(ifs, line);
        // Trim newline \r\n or \n
        if(line.size() > 0 && line[line.size() - 1] == '\n')
            line.erase(line.size() - 1);
        if(line.size() > 0 && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if(line.empty())
            continue;

        std::stringstream ss;
        std::string op;
        ss.str(line);
        ss >> op;

        if(op == "newmtl") {
            materials.insert(std::pair<std::string, Material>(name, m));
            m = Material();
            ss >> name;
            continue;
        }

        if(op == "Kd" || op == "albedo") {
            float r, g, b;
            ss >> r >> g >> b;
            m.albedo = glm::vec3(r, g, b);
            continue;
        }

        if(op == "map_Kd") {
            ss >> m.albedo_map;
            LoadTexture(m.albedo_map, dir);
            continue;
        }
    }
    materials.insert(std::pair<std::string, Material>(name, m));
}

void LoadObj(const char* objfile, std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& albedos) {
    std::vector<glm::vec3> gpoints;
    std::vector<glm::vec3> gnormals;
    std::vector<glm::vec2> gtexcoords;
    std::map<std::string, Material> gmaterials;

    Material cur_material;

    std::ifstream ifs(objfile);
    if(!ifs) {
        std::cerr << "[INFO] Cannot open file " << objfile << "." << std::endl;
        return;
    }

    while(!ifs.eof()) {
        std::string line;
        std::getline(ifs, line);
        // Trim newline \r\n or \n
        if(line.size() > 0 && line[line.size() - 1] == '\n')
            line.erase(line.size() - 1);
        if(line.size() > 0 && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if(line.empty())
            continue;

        std::stringstream ss;
        std::string op;
        ss.str(line);
        ss >> op;
        
        // 'v': vertex
        if(op == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            gpoints.emplace_back(x, y, z);
            continue;
        }

        // 'vn': normal
        if(op == "vn") {
            float x, y, z;
            ss >> x >> y >> z;
            gnormals.emplace_back(x, y, z);
            continue;
        }

        // 'vt': texcoord
        if(op == "vt") {
            float u, v;
            ss >> u >> v;
            gtexcoords.emplace_back(u, 1.0 - v);
            continue;
        }

        // face(要求三角形，要求包含v/vt/vn)
        if(op == "f") {
            int vid[3], tcid[3], nid[3];
            
            std::string str;
            std::getline(ss, str);
            sscanf(str.c_str(), "%d/%d/%d %d/%d/%d %d/%d/%d", &vid[0], &tcid[0], &nid[0], &vid[1], &tcid[1], &nid[1], &vid[2], &tcid[2], &nid[2]);
//            std::cout << vid[0] << " " << vid[1] << " " << vid[2] << std::endl;
            for(int i = 0; i < 3; i++) {
                points.push_back(gpoints[vid[i] - 1]);
                normals.push_back(gnormals[nid[i] - 1]);
                if(!cur_material.albedo_map.empty()) {
                    float u = gtexcoords[tcid[i] - 1].x;
                    float v = gtexcoords[tcid[i] - 1].y;
                    // TODO: 用(u, v)在albedo_map上采样，得到albedo放入albedos中
                    if(textures.find(cur_material.albedo_map) == textures.end()) {
                        std::cerr << "[ERROR] " << cur_material.albedo_map << " not found." << std::endl;
                        return;
                    }
                    albedos.push_back(textures[cur_material.albedo_map].LookUp(u, v));
                } else {
                    albedos.push_back(cur_material.albedo);
                }
            }
            continue;
        }

        if(op == "usemtl") {
            std::string mtlname;
            ss >> mtlname;
            auto it = gmaterials.find(mtlname);
            if(it != gmaterials.end()) {
                cur_material = it->second;
            } else {
                std::cerr << "[ERROR] Material not found." << std::endl;
                return;
            }
            continue;
        }

        if(op == "mtllib") {
            std::string mtlfile;
            ss >> mtlfile;

            std::string file(objfile);
            const char* dir = strrchr(file.c_str(), '/');
            if(dir > file.c_str() || file[0] == '/') {
                dir++;
            }
            LoadMtl(mtlfile.c_str(), file.substr(0, dir - file.c_str()).c_str(), gmaterials);
            continue;
        }
    }

}