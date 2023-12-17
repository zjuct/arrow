#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "texturemgr.hpp"
#include "shader.h"

struct material_t {
    material_t(TextureMgr* _texmgr = &default_texmgr) : texmgr(_texmgr) {
        name = "";
        for (int i = 0; i < 3; i++) {
            ambient[i] = 0.0f;
            diffuse[i] = 0.0f;
            specular[i] = 0.0f;
        }
        shininess = 1.0f;
        illum = 0;
        ambient_texname = "";
        diffuse_texname = "";
        specular_texname = "";
        bump_texname = "";

        albedo = glm::vec3(1.0f);
        albedo_texname = "";
        metallic = 0.0f;
        roughness = 0.0f;
    }

    void configShader(Shader* shader) {
        shader->use();
        if (!diffuse_texname.empty()) {
            int tex = texmgr->gettex(diffuse_texname, TEX_2D);
            if (tex < 0) {
                std::cerr << "[ERROR] No texture." << std::endl;
            }
            shader->setInt("material.diffuse", 0);      // 绑定到0号纹理单元
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);      // 将纹理数据绑定到纹理单元
        }
        if (!specular_texname.empty()) {
            int tex = texmgr->gettex(specular_texname, TEX_2D);
            if (tex < 0) {
                std::cerr << "[ERROR] No texture." << std::endl;
            }
            shader->setInt("material.specular", 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, tex);
        }
        if (!albedo_texname.empty()) {
            int tex = texmgr->gettex(albedo_texname, TEX_2D);
            if (tex < 0) {
                std::cerr << "[ERROR] No texture." << std::endl;
            }
            shader->setInt("material.albedo_map", 2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, tex);      // 将纹理数据绑定到纹理单元
        }
        shader->setBool("material.has_diffuse_map", !diffuse_texname.empty());
        shader->setBool("material.has_specular_map", !specular_texname.empty());
        shader->setFloat("material.shininess", shininess);
        shader->setvec3fv("material.Ka", ambient);
        shader->setvec3fv("material.Kd", diffuse);
        shader->setvec3fv("material.Ks", specular);

        shader->setvec3fv("material.albedo", glm::value_ptr(albedo));
        shader->setBool("material.has_albedo_map", !albedo_texname.empty());
        shader->setFloat("material.metallic", metallic);
        shader->setFloat("material.roughness", roughness);

        // Skybox
        if (!skybox_texname.empty()) {
            int tex = texmgr->gettex(skybox_texname, TEX_CUBEMAP);
            if (tex < 0) {
                std::cerr << "[ERROR] No texture." << std::endl;
            }
            shader->setInt("material.skybox_map", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
        }
    }

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

    TextureMgr* texmgr;
};

#endif