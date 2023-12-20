#ifndef _OBBGEN_H
#define _OBBGEN_H

#include <obb.h>

#include <glm/glm.hpp>

#include <vector>

Obb obbgen(const std::vector<glm::vec3>& vertices);

#endif