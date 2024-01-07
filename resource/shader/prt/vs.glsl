#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aPrecomputeLTR1;
layout (location = 4) in vec3 aPrecomputeLTR2;
layout (location = 5) in vec3 aPrecomputeLTR3;
layout (location = 6) in vec3 aPrecomputeLTG1;
layout (location = 7) in vec3 aPrecomputeLTG2;
layout (location = 8) in vec3 aPrecomputeLTG3;
layout (location = 9) in vec3 aPrecomputeLTB1;
layout (location = 10) in vec3 aPrecomputeLTB2;
layout (location = 11) in vec3 aPrecomputeLTB3;
//layout (location = 3) in mat3 aPrecomputeLTR;
//layout (location = 4) in mat3 aPrecomputeLTG;
//layout (location = 5) in mat3 aPrecomputeLTB;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

out vec3 PrecomputeLTR1;
out vec3 PrecomputeLTR2;
out vec3 PrecomputeLTR3;
out vec3 PrecomputeLTG1;
out vec3 PrecomputeLTG2;
out vec3 PrecomputeLTG3;
out vec3 PrecomputeLTB1;
out vec3 PrecomputeLTB2;
out vec3 PrecomputeLTB3;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightSpaceMatrix;


float dotmat3(vec3 L1, vec3 L2, vec3 L3, mat3 LT) {
    float ret = 0.0;
    ret += L1[0] * LT[0][0] + L1[1] * LT[0][1] + L1[2] * LT[0][2];
    ret += L2[0] * LT[1][0] + L2[1] * LT[1][1] + L2[2] * LT[1][2];
    ret += L3[0] * LT[2][0] + L3[1] * LT[2][1] + L3[2] * LT[2][2];

//    ret = L[0][0] * LT[0][0] + L[0][1] * LT[0][1] + L[0][2] * LT[0][2]
//        + L[1][0] * LT[1][0] + L[1][1] * LT[1][1] + L[1][2] * LT[1][2]
//        + L[2][0] * LT[2][0] + L[2][1] * LT[2][1] + L[2][2] * LT[2][2];
    return ret;
}


void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);

    gl_Position = projection * view * vec4(FragPos, 1.0);

    PrecomputeLTR1 = aPrecomputeLTR1;
    PrecomputeLTR2 = aPrecomputeLTR2;
    PrecomputeLTR3 = aPrecomputeLTR3;
    PrecomputeLTG1 = aPrecomputeLTG1;
    PrecomputeLTG2 = aPrecomputeLTG2;
    PrecomputeLTG3 = aPrecomputeLTG3;
    PrecomputeLTB1 = aPrecomputeLTB1;
    PrecomputeLTB2 = aPrecomputeLTB2;
    PrecomputeLTB3 = aPrecomputeLTB3;

//    float R = dotmat3(aPrecomputeLTR1, aPrecomputeLTR2, aPrecomputeLTR3, PrecomputeL[0]);
//    float G = dotmat3(aPrecomputeLTG1, aPrecomputeLTG2, aPrecomputeLTG3, PrecomputeL[1]);
//    float B = dotmat3(aPrecomputeLTB1, aPrecomputeLTB2, aPrecomputeLTB3, PrecomputeL[2]);
//    Radiance = vec3(R, G, B);
//    Radiance = vec3(aPrecomputeLTR1[0], aPrecomputeLTR2[0], aPrecomputeLTR3[0]);
//    mat3 PrecomputeLTR(aPrecomputeLTR1, aPrecomputeLTR2, aPrecomputeLTR3);
//    mat3 PrecomputeLTG(aPrecomputeLTG1, aPrecomputeLTG2, aPrecomputeLTG3);
//    mat3 PrecomputeLTB(aPrecomputeLTB1, aPrecomputeLTB2, aPrecomputeLTB3);
//    Radiance = vec3(dotmat3(PrecomputeLTR1, PrecomputeLTR2, PrecomputeLTR3, PrecomputeL[0]), dotmat3(PrecomputeLTG1, PrecomputeLTG2, PrecomputeLTG3, PrecomputeL[1]), dotmat3(PrecomputeLTB1, PrecomputeLTB2, PrecomputeLTB3, PrecomputeL[2]));
}