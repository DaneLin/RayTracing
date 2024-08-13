#include "ray.hpp"

Ray Ray::ObjectFromWorld(const glm::mat4 & object_from_world) const
{
    glm::vec4 o = object_from_world * glm::vec4(origin, 1.0f);
    glm::vec4 d = object_from_world * glm::vec4(direction, 0.0f);
    return Ray{glm::vec3(o), glm::vec3(d)};
}
