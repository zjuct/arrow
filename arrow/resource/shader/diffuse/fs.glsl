#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse_map;
    bool has_diffuse_map;

    vec3 diffuse;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform Material material;

void main() {
    FragColor = texture(material.diffuse_map, TexCoords);
}