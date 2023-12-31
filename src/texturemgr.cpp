#include <texturemgr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glad/glad.h"
#include <iostream>
#include <vector>

TextureMgr::TextureMgr() {

}

TextureMgr* TextureMgr::getInstance() {
    static TextureMgr texmgr;
    return &texmgr;
}

bool TextureMgr::load(const std::string& path, TexType type) {
    if(texs.find(path) != texs.end())
        return true;

    switch(type) {
        case TEX_2D:
            return load_2D(path);
        case TEX_CUBEMAP:
            return load_CubeMap(path);
        default:
            std::cout << "[ERROR] Unknown texture type." << std::endl;
    }
    return false;
}

bool TextureMgr::load_2D(const std::string& file) {
    unsigned int texture;
    glGenTextures(1, &texture);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format;
        if(nrChannels == 1)
            format = GL_RED;
        else if(nrChannels == 3) 
            format = GL_RGB;
        else if(nrChannels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 为当前绑定的纹理对象设置环绕、过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        texs.insert(std::pair<std::string, unsigned int>(file, texture));
        return true;
    } else {
        std::cerr << "Failed to load texture " << file << std::endl;
        return false;
    }
}

bool TextureMgr::load_CubeMap(const std::string& path) {
    std::vector<std::string> faces = {
//        "/right.jpg",
//        "/left.jpg",
//        "/top.jpg",
//        "/bottom.jpg",
//        "/front.jpg",
//        "/back.jpg",
        "/posx.jpg",
        "/negx.jpg",
        "/posy.jpg",
        "/negy.jpg",
        "/posz.jpg",
        "/negz.jpg",
    };
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    
    int width, height, nrChannels;
    for(unsigned int i = 0; i < faces.size(); i++) {
        unsigned char* data = stbi_load((path + faces[i]).c_str(), &width, &height, &nrChannels, 0);
        if(data) {
            GLenum format;
            if(nrChannels == 1)
                format = GL_RED;
            else if(nrChannels == 3)
                format = GL_RGB;
            else if(nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap texture failed to load at path: " << path + faces[i] << std::endl;
            stbi_image_free(data);
            return false;
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    texs.insert(std::pair<std::string, unsigned int>(path, texture));

    return true;
}

