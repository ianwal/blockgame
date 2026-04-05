#pragma once

#include <glm/glm.hpp>

namespace bg
{

constexpr glm::vec3 lerp(glm::vec3 prev, glm::vec3 current, float t)
{
    float x = prev.x + ((current.x - prev.x) * t);
    float y = prev.y + ((current.y - prev.y) * t);
    float z = prev.z + ((current.z - prev.z) * t);
    return glm::vec3{ x, y, z };
}

} // namespace bg
