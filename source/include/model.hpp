#pragma once

#include "triangle.hpp"
#include <filesystem>

class Model : public Shape
{
public:
    Model(const std::filesystem::path& filename);

    Model(const std::vector<Triangle>& triangles)
        : triangles(triangles) {}

    std::optional<HitInfo> intersect(const Ray& ray, float t_min, float t_max) const override;

private:
    std::vector<Triangle> triangles;
};