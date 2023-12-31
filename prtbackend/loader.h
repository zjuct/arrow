#ifndef _LOADER_H
#define _LOADER_H

#include <vector>
#include <string>

#include <glm/glm.hpp>

std::vector<float*> LoadCubemapImages(const std::string& cubemapDir, int& width, int& height, int& channel);

void LoadObj(const char* objfile, std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, std::vector<glm::vec3>& albedos);

#endif