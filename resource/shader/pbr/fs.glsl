#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

struct PBRMaterial {
    sampler2D albedo_map;
    bool has_albedo_map;
    vec3 albedo;       // 固有色(反射率)，用于计算BRDF中的漫反射项
    
    float metallic;
    float roughness;
};

struct DirLight {
    bool enable;

    vec3 direction;
    vec3 intensity;
};

struct PointLight {
    bool enable;

    vec3 position;
    vec3 intensity;
};

uniform vec3 viewPos;
uniform PBRMaterial material;
uniform DirLight dirLight;
uniform PointLight pointLight[4];

const float PI = 3.14159265359;

vec3 cookTorrance(vec3 lightDir, vec3 viewDir, vec3 normal, PBRMaterial material);

float DistributionGGX(vec3 N, vec3 H, float a) {
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k) {
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float G1 = GeometrySchlickGGX(NdotV, k);
    float G2 = GeometrySchlickGGX(NdotL, k);

    return G1 * G2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CookTorrance(vec3 L, vec3 V, vec3 N, vec3 F0, float roughness, float metallic, vec3 albedo, vec3 radiance) {
    float VdotN = max(dot(V, N), 0.0);
    float LdotN = max(dot(L, N), 0.0);

    vec3 H = normalize(L + V);
    float VdotH = max(dot(V, H), 0.0);

    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(VdotH, F0);

    vec3 num = D * G * F;
    float denom = max(4.0 * VdotN * LdotN, 0.0001);
    vec3 sepc = num / denom;

    vec3 kS = F;
    vec3 kD = (1.0 - kS) * (1.0 - metallic);

    vec3 radiance_out = (kD * albedo / PI + sepc) * radiance * LdotN;
    return radiance_out;
}

void main() {
    // Gamma校正
    vec3 albedo = material.has_albedo_map ? pow(texture2D(material.albedo_map, TexCoords).rgb, vec3(2.2)) : material.albedo;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 normal = normalize(Normal);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, material.metallic);

    vec3 Lo = vec3(0.0);
    if(dirLight.enable) {
        vec3 lightDir = -normalize(dirLight.direction);
        vec3 radiance = dirLight.intensity;     // 定向光没有衰减
        Lo += CookTorrance(lightDir, viewDir, normal, F0, material.roughness, material.metallic, albedo, radiance);
    }
    
    for(int i = 0; i < 4; i++) {
        if(pointLight[i].enable) {
            vec3 lightDir = normalize(pointLight[i].position - FragPos);
            float distance = length(pointLight[i].position - FragPos);
            float attenuation = 1.0 / (distance * distance);
            vec3 radiance = pointLight[i].intensity * attenuation;
            Lo += CookTorrance(lightDir, viewDir, normal, F0, material.roughness, material.metallic, albedo, radiance);
        }
    }

    // CookTorrance最后会乘一个LdotN，导致背光面全黑，只考虑了直接光照
    vec3 ambient = vec3(0.03) * albedo;

    vec3 color = Lo + ambient;
    // Gamma校正
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 
    FragColor = vec4(color, 1.0);
//    FragColor = vec4(pointLight[0].intensity, 1.0);
}

float backmannDistribution(float NdotH, float roughness) {
    float alpha2 = roughness * roughness;
    float x = NdotH * NdotH * (alpha2 - 1.0) + 1.0;
    return alpha2 / (3.1415926 * x * x); 
}