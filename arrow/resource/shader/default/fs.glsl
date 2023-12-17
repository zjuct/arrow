#version 330 core
out vec4 FragColor;

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

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
// 支持一个平行光源和一个点光源
uniform DirLight dirLight;
uniform PointLight pointLight;

uniform bool has_normal;
uniform bool has_texcoord;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0f, 0.0f, 0.0f);
    if(dirLight.enable) {
        result += CalcDirLight(dirLight, norm, viewDir);
    }
    if(pointLight.enable) {
        result += CalcPointLight(pointLight, norm, FragPos, viewDir);     
    }

    FragColor = vec4(result, 1.0);
//    FragColor = texture(material.diffuse, TexCoords);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // 合并结果
    vec3 ambient  = light.ambient * (material.has_diffuse_map ? texture(material.diffuse, TexCoords).rgb : material.Ka);
    vec3 diffuse  = light.diffuse * diff * (material.has_diffuse_map ? texture(material.diffuse, TexCoords).rgb : material.Kd);
    vec3 specular = light.specular * spec * (material.has_specular_map ? texture(material.specular, TexCoords).rgb : material.Ks);
    return (ambient + diffuse + specular);
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
    vec3 ambient  = light.ambient  * (material.has_diffuse_map ? texture(material.diffuse, TexCoords).rgb : material.Ka);
    vec3 diffuse  = light.diffuse  * diff * (material.has_diffuse_map ? texture(material.diffuse, TexCoords).rgb : material.Kd);
    vec3 specular = light.specular * spec * (material.has_specular_map ? texture(material.specular, TexCoords).rgb : material.Ks);

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}