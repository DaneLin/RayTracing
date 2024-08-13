#pragma once

#include "shape.hpp"

struct ShapeInstance
{
    const Shape* shape;
    glm::mat4 world_from_object;
    glm::mat4 object_from_world;
};

class Scene : public Shape
{
public:
    Scene() = default;
    Scene(const Scene &) = delete;
    Scene(Scene &&) = delete;
    Scene &operator=(const Scene &) = delete;
    Scene &operator=(Scene &&) = delete;

    void addShape(const Shape *shape, 
        const glm::vec3& position = glm::vec3(0.f, 0.f, 0.f), 
        const glm::vec3& rotation = glm::vec3(0.f, 0.f, 0.f), 
        const glm::vec3& scale = glm::vec3(1.f, 1.f, 1.f));

    virtual std::optional<HitInfo> intersect(
        const Ray &ray,
        float t_min = 1e-5,
        float t_max = std::numeric_limits<float>::infinity()) const override;

private:
    std::vector<ShapeInstance> instances;
};