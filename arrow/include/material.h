#ifndef _MATERIAL_H
#define _MATERIAL_H

#include <shader.h>

#include <glm/glm.hpp>

struct material_t {
    material_t();
    void configShader(Shader* shader);

    std::string name;

    float ambient[3];       // Ka
    float diffuse[3];       // Kd
    float specular[3];      // Ks
    float shininess;        // Ns，指数
    int illum;              // illum

    // index into TextureMgr
    std::string ambient_texname;    // map_Ka
    std::string diffuse_texname;    // map_Kd
    std::string specular_texname;   // map_Ks
    std::string bump_texname;       // map_bump

    // PBR
    glm::vec3 albedo;
    std::string albedo_texname;
    float metallic;
    float roughness;

    // SkyBox
    std::string skybox_texname;
};

#endif