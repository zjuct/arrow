#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

in vec3 PrecomputeLTR1;
in vec3 PrecomputeLTR2;
in vec3 PrecomputeLTR3;
in vec3 PrecomputeLTG1;
in vec3 PrecomputeLTG2;
in vec3 PrecomputeLTG3;
in vec3 PrecomputeLTB1;
in vec3 PrecomputeLTB2;
in vec3 PrecomputeLTB3;

uniform mat3 PrecomputeL[3];

float dotmat3(vec3 L1, vec3 L2, vec3 L3, mat3 LT) {
    float ret = 0.0;
    ret += L1[0] * LT[0][0] + L1[1] * LT[0][1] + L1[2] * LT[0][2];
    ret += L2[0] * LT[1][0] + L2[1] * LT[1][1] + L2[2] * LT[1][2];
    ret += L3[0] * LT[2][0] + L3[1] * LT[2][1] + L3[2] * LT[2][2];

    return ret;
}


void main() {
    float R = 2.0 * dotmat3(PrecomputeLTR1, PrecomputeLTR2, PrecomputeLTR3, PrecomputeL[0]);
    float G = 2.0 * dotmat3(PrecomputeLTG1, PrecomputeLTG2, PrecomputeLTG3, PrecomputeL[1]);
    float B = 2.0 * dotmat3(PrecomputeLTB1, PrecomputeLTB2, PrecomputeLTB3, PrecomputeL[2]);
    FragColor = vec4(R, G, B, 1.0);
}
