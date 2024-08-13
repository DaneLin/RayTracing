#pragma once

#include <glm/glm.hpp>

struct Material
{
    glm::vec3 albedo{ 1, 1, 1 };
    bool bIsSpecular=false;
    glm::vec3 emissive{0.0f,0.f, 0.f};
};