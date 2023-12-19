#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    bool has_diffuse_map;

    vec3 albedo;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform Material material;

void main() {
    FragColor = material.has_diffuse_map ? texture(material.diffuse, TexCoords) : vec4(material.albedo, 1.0f);
}