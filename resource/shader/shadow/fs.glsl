
#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

struct Material {
    sampler2D diffuse;
    bool has_diffuse_map;
    sampler2D specular;
    bool has_specular_map;

    float shininess;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
};

struct DirLight {
    bool enable;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    bool enable;
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewPos;
uniform Material material;
// 支持一个平行光源和一个点光源
uniform DirLight dirLight;
uniform PointLight pointLight;

uniform sampler2D shadowMap;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = (currentDepth - bias) > closestDepth  ? 1.0 : 0.0;
    return shadow;
}

void main() {
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    if(dirLight.enable) {
        result += CalcDirLight(dirLight, norm, viewDir);
    }
//    if(pointLight.enable) {
//        result += CalcPointLight(pointLight, norm, fs_in.FragPos, viewDir);     
//    }

    FragColor = vec4(result, 1.0);

//    vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
//    float currentDepth = projCoords.z;

//    vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
//    float depth = texture(shadowMap, projCoords.xy).r;
//    FragColor = vec4(depth, depth, depth, 1.0f);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 5.0);
    // 合并结果
    vec3 ambient  = light.ambient * (material.has_diffuse_map ? texture(material.diffuse, fs_in.TexCoords).rgb : material.Ka);
    vec3 diffuse  = light.diffuse * diff * (material.has_diffuse_map ? texture(material.diffuse, fs_in.TexCoords).rgb : material.Kd);
    vec3 specular = light.specular * spec * (material.has_specular_map ? texture(material.specular, fs_in.TexCoords).rgb : material.Ks);
    
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
    return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 衰减
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                 light.quadratic * (distance * distance));    
    // 合并结果
    vec3 ambient  = light.ambient  * (material.has_diffuse_map ? texture(material.diffuse, fs_in.TexCoords).rgb : material.Ka);
    vec3 diffuse  = light.diffuse  * diff * (material.has_diffuse_map ? texture(material.diffuse, fs_in.TexCoords).rgb : material.Kd);
    vec3 specular = light.specular * spec * (material.has_specular_map ? texture(material.specular, fs_in.TexCoords).rgb : material.Ks);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
    return ambient + (1.0 - shadow) * (diffuse + specular);
}