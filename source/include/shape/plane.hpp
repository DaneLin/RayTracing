#pragma once

#include "shape.hpp"

struct Plane : public Shape
{
    Plane(const glm::vec3& point, const glm::vec3& normal)
        : point(point), normal(glm::normalize(normal)) {}

    virtual  std::optional<HitInfo> intersect(
        const Ray& ray,
        float t_min,
        float t_max
    ) const override;

    // 平面由一个点和法向量定义
    glm::vec3 point;
    glm::vec3 normal;
};