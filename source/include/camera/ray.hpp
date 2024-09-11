#pragma once

#include <glm/glm.hpp>
#include "../shape/material.hpp"

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    glm::vec3 hit(float t) const { return origin + t * direction; }

    Ray ObjectFromWorld(const glm::mat4& object_from_world) const;

#ifdef WITH_DEBUG_INFO
    mutable size_t bounds_test_count = 0;
    mutable size_t triangle_test_count = 0;
#endif
};

struct HitInfo
{
    float t;
    glm::vec3 hit_point;
    glm::vec3 normal;
    const Material *material = nullptr;


};