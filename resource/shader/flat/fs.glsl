#version 330 core
out vec4 FragColor;

uniform sampler2D tex;
uniform bool has_texture;
uniform vec3 color;

in vec2 TexCoords;

void main() {
    FragColor = has_texture ? texture(tex, TexCoords) : vec4(color, 1.0f);
}