#include "accelerate/bounds.hpp"

#include <cmath>

bool Bounds::hasIntersection(const Ray & ray, float t_min, float t_max) const
{
    glm::vec3 t1 = (b_min - ray.origin) / (ray.direction);
    glm::vec3 t2 = (b_max - ray.origin) / (ray.direction);
    glm::vec3 tmin = glm::min(t1, t2);
    glm::vec3 tmax = glm::max(t1, t2);

    float near = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
    float far = glm::min(tmax.x, glm::min(tmax.y, tmax.z));

    if (near <= t_min && far >= t_max)
    {
        return false;
    }
    return glm::max(near, t_min) <= glm::min(t_max, far);
}