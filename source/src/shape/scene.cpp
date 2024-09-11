#include "shape/scene.hpp"
#include <glm/gtc/matrix_transform.hpp>

#define EPS 1e-5f

void Scene::addShape(const Shape& shape, const Material &material,const glm::vec3 & position, const glm::vec3 & rotation, const glm::vec3 & scale)
{
    glm::mat4 world_from_object = glm::translate(glm::mat4(1.f), position)
        * glm::rotate(glm::mat4(1.f), glm::radians(rotation.z), glm::vec3(0, 0, 1))
        * glm::rotate(glm::mat4(1.f), glm::radians(rotation.y), glm::vec3(0, 1, 0))
        * glm::rotate(glm::mat4(1.f), glm::radians(rotation.x), glm::vec3(1, 0, 0))
        * glm::scale(glm::mat4(1.f), scale);
    instances.push_back(ShapeInstance{ shape, material, world_from_object, glm::inverse(world_from_object)});
}

std::optional<HitInfo> Scene::intersect(const Ray &ray, float t_min, float t_max) const
{
	return scene_bvh.intersect(ray, t_min, t_max);
}
