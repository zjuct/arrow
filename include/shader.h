#ifndef _SHADER_H
#define _SHADER_H

#include "glad/glad.h" // 包含glad来获取所有的必须OpenGL头文件

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    static void initShader();

    // 程序ID
    unsigned int ID;

    // 构造器读取并构建着色器
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    // 使用/激活程序
    void use();
    // uniform工具函数
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setmat4fv(const std::string &name, int transpose, GLfloat* value) const;
    void setvec4fv(const std::string &name, GLfloat* value) const;
    void setvec3fv(const std::string &name, GLfloat* value) const;
    void setvec3(const std::string& name, GLfloat x, GLfloat y, GLfloat z) const;
};

extern Shader* default_shader;
extern Shader* diffuse_shader;
extern Shader* pbr_shader;
extern Shader* skybox_shader;
extern Shader* player_shader;
extern Shader* segment_shader;
extern Shader* flat_shader;
extern Shader* shadowmap_shader;
extern Shader* shadow_shader;
extern Shader* debug_shader;

#endif