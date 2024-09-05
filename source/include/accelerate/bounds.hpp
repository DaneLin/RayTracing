#pragma once

#include "glm/glm.hpp"
#include "camera/ray.hpp"

struct Bounds{

    Bounds() : b_min(glm::vec3(std::numeric_limits<float>::max())), b_max(glm::vec3(std::numeric_limits<float>::min())) {}
    Bounds(const glm::vec3& b_min, const glm::vec3& b_max) : b_min(b_min), b_max(b_max) {}

    void expand(const glm::vec3 &pos)
    {
        b_min = glm::min(pos, b_min);
        b_max = glm::max(pos, b_max);
    }

    bool hasIntersection(const Ray& ray, float t_min, float t_max) const;


    glm::vec3 b_min;
    glm::vec3 b_max;
};