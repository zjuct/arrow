#include <prt.h>
#include <spherical_harmonics.h>
#include <loader.h>
#include <scene.h>
#include <defs.h>

#include <vector>
#include <fstream>
#include <iostream>
#include <random>

#include <glm/glm.hpp>

#define ALBEDO_ENABLE 0

static constexpr int SHOrder = 2;
static constexpr int m_SampleCount = 100;

const glm::vec3 cubemapFaceDirections[6][3] = {
    {{0, 0, 1}, {0, -1, 0}, {-1, 0, 0}},  // negx
    {{0, 0, 1}, {0, -1, 0}, {1, 0, 0}},   // posx
    {{1, 0, 0}, {0, 0, -1}, {0, -1, 0}},  // negy
    {{1, 0, 0}, {0, 0, 1}, {0, 1, 0}},    // posy
    {{-1, 0, 0}, {0, -1, 0}, {0, 0, -1}}, // negz
    {{1, 0, 0}, {0, -1, 0}, {0, 0, 1}},   // posz
};


float CalcPreArea(float x, float y) {
    return std::atan2(x * y, std::sqrt(x * x + y * y + 1.0));
}

// 计算Cubemap微元在球面上的面积
float CalcArea(int _u, int _v, int width, int height) {
    // transform from [0..res - 1] to [- (1 - 1 / res) .. (1 - 1 / res)]
    // ( 0.5 is for texel center addressing)
    float u = (2.0 * (_u + 0.5) / width) - 1.0;
    float v = (2.0 * (_v + 0.5) / height) - 1.0;

    // shift from a demi texel, mean 1.0 / size  with u and v in [-1..1]
    float invResolutionW = 1.0 / width;
    float invResolutionH = 1.0 / height;

    // u and v are the -1..1 texture coordinate on the current face.
    // get projected area for this texel
    float x0 = u - invResolutionW;
    float y0 = v - invResolutionH;
    float x1 = u + invResolutionW;
    float y1 = v + invResolutionH;
    float angle = CalcPreArea(x0, y0) - CalcPreArea(x0, y1) -
                  CalcPreArea(x1, y0) + CalcPreArea(x1, y1);

    return angle;
}

std::vector<glm::vec3> PrecomputeLight(const std::vector<float*>& images,
                                       int width, int height, int channel) {
                                    
    // 计算Cubemap上所有点对应的方向
    std::vector<glm::vec3> cubemapDirs;
    cubemapDirs.reserve(6 * width * height);
    for(int i = 0; i < 6; i++) {
        glm::vec3 faceDirX = cubemapFaceDirections[i][0];
        glm::vec3 faceDirY = cubemapFaceDirections[i][1];
        glm::vec3 faceDirZ = cubemapFaceDirections[i][2];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float u = 2 * ((x + 0.5) / width) - 1;
                float v = 2 * ((y + 0.5) / height) - 1;
                glm::vec3 dir = glm::normalize(faceDirX * u + faceDirY * v + faceDirZ);
                cubemapDirs.push_back(dir);
            }
        }
    }

    constexpr int SHNum = (SHOrder + 1) * (SHOrder + 1);
    std::vector<glm::vec3> SHCoeffiecents(SHNum);
    for (int i = 0; i < SHNum; i++)
    {
        SHCoeffiecents[i] = glm::vec3(0.0f);
    }
    float sumWeight = 0.0f;
    // 计算黎曼和
    for(int i = 0; i < 6; i++) {
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                glm::vec3 dir = cubemapDirs[i * width * height + y * width + x];
                int index = (y * width + x) * channel;
                glm::vec3 light_emit(images[i][index + 0], images[i][index + 1], images[i][index + 2]);

                float dwi = CalcArea(x, y, width, height);
                dir = glm::normalize(dir);
                for(int l = 0; l <= SHOrder; l++) {
                    for(int m = -l; m <= l; m++) {
                        SHCoeffiecents[SHGetIndex(l, m)] += light_emit * EvalSH(l, m, dir) * dwi;
                    }
                }
            }
        }
    }
    return SHCoeffiecents;
}

std::vector<std::array<std::vector<float>, 3>> PrecomputeLightTransport(const Scene& scene) {
    std::vector<std::array<std::vector<float>, 3>> SHTransportCoeffs;
    for(int i = 0; i < scene.nr_triangles * 3; i++) {
        std::cout << i << std::endl;
        glm::vec3 point = scene.points[i];
        glm::vec3 normal = scene.normals[i];
#if ALBEDO_ENABLE
        glm::vec3 albedo = scene.albedos[i];
#else
        glm::vec3 albedo(1.0f);
#endif

        // Diffuse unshadowed
        auto shFunc = [&](float phi, float theta, int& cnt) -> float {
            glm::vec3 d = ToVector(phi, theta);
            float cos_theta = glm::dot(d, normal);
            Intersect intersect;
            if(cos_theta > 0.0f) {
                // 光线与场景求交
                Ray ray(point, d);
                if(!scene.RayIntersect(ray, intersect)) {
                    cnt++;
                    return cos_theta / M_PI;
                }
//                std::cout << "t: " << intersect.t << std::endl;
//                std::cout << "bary: " << intersect.bary[0] << " " << intersect.bary[1] << " " << intersect.bary[2] << std::endl;
            }
            // 被挡住
            return 0.0f;
        };
        auto shCoeff = ProjectFunction(SHOrder, shFunc, m_SampleCount);
        std::array<std::vector<float>, 3> shCoeffColor;
        for(int j = 0; j < 3; j++) {
            for(int k = 0; k < shCoeff.size(); k++) {
                shCoeffColor[j].push_back(albedo[j] * shCoeff[k]);
            }
        }
//        for(int j = 0; j < 3; j++) {
//            for(int k = 0; k < shCoeff.size(); k++) {
//                std::cout << shCoeffColor[j][k] << " ";
//            }
//            std::cout << std::endl;
//        }
        SHTransportCoeffs.push_back(shCoeffColor);
    }
    return SHTransportCoeffs;
}

std::vector<std::array<std::vector<float>, 3>> PrecomputeLightTransportBounce(const Scene& scene, const std::vector<std::array<std::vector<float>, 3>>& SHLT_before) {
    std::vector<std::array<std::vector<float>, 3>> SHLT(SHLT_before);
    auto interrefl = [&](int idx, const glm::vec3& v, const glm::vec3& n, int order, int sample_count) {
        const int sample_side = static_cast<int>(floor(sqrt(sample_count)));

        // generate sample_side^2 uniformly and stratified samples over the sphere
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> rng(0.0, 1.0);
        for(int t = 0; t < sample_side; t++) {
            for(int p = 0; p < sample_side; p++) {
                float alpha = (t + rng(gen)) / sample_side;
                float beta = (p + rng(gen)) / sample_side;
                float phi = 2.0 * M_PI * beta;
                float theta = acos(2.0 * alpha - 1.0);

                glm::vec3 d = ToVector(phi, theta);
                Intersect inter;
                bool intersected = scene.RayIntersect(Ray(v, d), inter);
                float V = (1 - intersected ? 0 : 1) * glm::dot(d, n);

                if(V > 0.0f) {
                    std::array<float, 3> bary = inter.bary;
                    int tri_idx = inter.tri_idx;
                    for(int i = 0; i < 3; i++) {
                        for(int j = 0; j < GetCoefficientCount(order); j++) {
                            float refSH = bary[0] * SHLT_before[3 * tri_idx][i][j]
                                        + bary[1] * SHLT_before[3 * tri_idx + 1][i][j]
                                        + bary[2] * SHLT_before[3 * tri_idx + 2][i][j];
                            SHLT[idx][i][j] += refSH * V;
                        }
                    }
                }
            }
        }

    };

    for(int i = 0; i < scene.nr_triangles * 3; i++) {
        std::cout << i << std::endl;
        glm::vec3 point = scene.points[i];
        glm::vec3 normal = scene.normals[i];
        interrefl(i, point, normal, SHOrder, m_SampleCount);
    }
    return SHLT;
}

const char* precompute_L = "resource/assets/CornellBox/lighting.txt";
const char* precomupte_LT = "resource/assets/CornellBox/lighttransport.txt";

constexpr int m_bounce = 0;

void PRTPrecompute(const char* sceneobj, const char* skybox) {
    Scene scene(sceneobj);

    // Lighting
    int width = 0, height = 0, channels = 0;
    std::vector<float*> cubemapImages = LoadCubemapImages(skybox, width, height, channels);
    std::vector<glm::vec3> m_LightCoeffs = PrecomputeLight(cubemapImages, width, height, channels);
    std::ofstream lighting_ofs(precompute_L);
    for(auto& coeff : m_LightCoeffs) {
        lighting_ofs << coeff.r << " " << coeff.g << " " << coeff.b << std::endl;
    }
    lighting_ofs.close();

    // LightTransport
    auto SHCoeffLT = PrecomputeLightTransport(scene);

    // Bounce
    for(int b = 1; b <= m_bounce; b++) {
        SHCoeffLT = PrecomputeLightTransportBounce(scene, SHCoeffLT);
    }

    std::ofstream ofs(precomupte_LT);
    for(int i = 0; i < SHCoeffLT.size(); i++) {
        for(int j = 0; j < 3; j++) {
            for(int k = 0; k < SHCoeffLT[i][j].size(); k++) {
                ofs << SHCoeffLT[i][j][k] << " ";
            }
            ofs << std::endl;
        }
    }
    ofs.close();
}