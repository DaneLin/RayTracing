#pragma once

#include <glm/glm.hpp>

struct RGB
{

    RGB(int red, int green, int blue) : r(red), g(green), b(blue) {}

    RGB(const glm::vec3& color)
    {
        r = glm::clamp<int>(glm::pow(color.x, 1.0 / 2.2) * 255, 0, 255);
        g = glm::clamp<int>(glm::pow(color.y, 1.0 / 2.2) * 255, 0, 255);
        b = glm::clamp<int>(glm::pow(color.z, 1.0 / 2.2) * 255, 0, 255);
    }


    operator glm::vec3() const {
        return glm::vec3 {
            glm::pow(r / 255.f, 2.2),
            glm::pow(g / 255.f, 2.2),
            glm::pow(b / 255.f, 2.2),
        };
    }

    int r, g, b;
};