#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

struct Material {
    samplerCube skybox_map;
};

uniform Material material;

void main()
{    
    FragColor = texture(material.skybox_map, TexCoords);
//    FragColor = vec4(TexCoords, 1.0f);
//    FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}