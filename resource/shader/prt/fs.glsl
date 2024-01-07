#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

in vec3 PrecomputeLTR1;
in vec3 PrecomputeLTR2;
in vec3 PrecomputeLTR3;
in vec3 PrecomputeLTG1;
in vec3 PrecomputeLTG2;
in vec3 PrecomputeLTG3;
in vec3 PrecomputeLTB1;
in vec3 PrecomputeLTB2;
in vec3 PrecomputeLTB3;

struct DirLight {
    bool enable;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform mat3 PrecomputeL[3];

uniform sampler2D shadowMap;
uniform bool PCF_enable;
uniform DirLight dirLight;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    if(PCF_enable) {
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 9.0;
    } else {
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
    }
    return shadow;
}

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

    vec3 lightDir = normalize(-dirLight.direction);
    vec3 normal = normalize(Normal);
    float shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
    R = (1.0 - 0.5 * shadow) * R;
    G = (1.0 - 0.5 * shadow) * G;
    B = (1.0 - 0.5 * shadow) * B;
    FragColor = vec4(R, G, B, 1.0);
}
