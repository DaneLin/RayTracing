#pragma once

#include "shape.hpp"

struct Triangle : public Shape
{
    Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
        const glm::vec3& n0, const glm::vec3& n1, const glm::vec3& n2)
        : p0(p0), p1(p1), p2(p2), n0(n0), n1(n1), n2(n2) {}

    Triangle(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
        : p0(p0), p1(p1), p2(p2)
    {
        auto ab = p1 - p0;
        auto ac = p2 - p0;
        n0 = glm::normalize(glm::cross(ab, ac));
        n1 = n0;
        n2 = n0;
    }

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
    Bounds getBounds() const override
    {
        Bounds bound{};
        bound.expand(p0);
		bound.expand(p1);
		bound.expand(p2);
		return bound;
    }

    glm::vec3 p0, p1, p2;
    glm::vec3 n0, n1, n2;
};