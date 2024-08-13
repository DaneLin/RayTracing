#include "scene.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>




void Scene::addShape(const Shape * shape, const glm::vec3 & position, const glm::vec3 & rotation, const glm::vec3 & scale)
{
    glm::mat4 world_from_object = glm::translate(glm::mat4(1.f), position)
        * glm::rotate(glm::mat4(1.f), glm::radians(rotation.z), glm::vec3(0, 0, 1))
        * glm::rotate(glm::mat4(1.f), glm::radians(rotation.y), glm::vec3(0, 1, 0))
        * glm::rotate(glm::mat4(1.f), glm::radians(rotation.x), glm::vec3(1, 0, 0))
        * glm::scale(glm::mat4(1.f), scale);
    instances.push_back({ shape, world_from_object, glm::inverse(world_from_object)});
}

std::optional<HitInfo> Scene::intersect(const Ray &ray, float t_min, float t_max) const
{
    std::optional<HitInfo> closest_hit_info{};
    const ShapeInstance* closest_instance = nullptr;
    
    // 此时传入的光线是世界空间中的，而我们ShapeInstance中shape计算交点是在物体空间中的，所以需要将光线转换到物体空间中
    for (const auto& instance : instances)
    {
        auto ray_object = ray.ObjectFromWorld(instance.object_from_world);
        auto hit_info = instance.shape->intersect(ray_object,t_min, t_max);

        if (hit_info.has_value())
        {
            if (hit_info.value().t < t_max)
            {
                t_max = hit_info.value().t;
                closest_hit_info = hit_info;
                closest_instance = &instance;
            }
        }
    }
    // 我们最终得到的结果是在物体空间中的，所以需要将其转换到世界空间中
    if (closest_instance)
    {
        closest_hit_info.value().hit_point = glm::vec3(closest_instance->world_from_object * glm::vec4(closest_hit_info.value().hit_point, 1.0f));
        // 需要注意的是，法线的变换需要使用到法线矩阵，即世界空间到物体空间的转置逆矩阵，这里我们已经有了物体空间到世界空间的逆矩阵，所以我们可以直接使用它的转置矩阵
        closest_hit_info->normal = glm::transpose(closest_instance->object_from_world) * glm::vec4(closest_hit_info->normal, 0.f);
    }

    return closest_hit_info;
}
