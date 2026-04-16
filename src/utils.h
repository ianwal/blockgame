#pragma once

#include <glm/glm.hpp>

// Include GLM experimental headers to be used transitively by other files because adding this define in every file is
// annoying and it adds extra maintenance.
// clang-format off
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>
// clang-format on

namespace bg
{

[[nodiscard]] constexpr float lerp(float prev, float current, float t)
{
    return prev + ((current - prev) * t);
}

[[nodiscard]] constexpr glm::vec3 lerp(glm::vec3 prev, glm::vec3 current, float t)
{
    float x = prev.x + ((current.x - prev.x) * t);
    float y = prev.y + ((current.y - prev.y) * t);
    float z = prev.z + ((current.z - prev.z) * t);
    return glm::vec3{ x, y, z };
}

[[nodiscard]] constexpr bool isZeroVector(glm::vec3 const& v)
{
    return v.x == 0.0f && v.y == 0.0f && v.z == 0.0f;
}

/// Round the given vector to zero if its length is near zero.
constexpr void roundToZero(glm::vec3& v)
{
    if (!isZeroVector(v)) {
        if (glm::length(v) < 0.02) {
            v = glm::vec3{ 0.0f, 0.0f, 0.0f };
        }
    }
}

/// Scale a vector to a specific length without changing its direction.
[[nodiscard]] constexpr glm::vec3 setLength(glm::vec3 const& v, float length)
{
    float mag = glm::length(v);
    if (mag == 0.0f) {
        return glm::vec3(0.0f);
    }
    return (v / mag) * length;
}

/// Clamp the vector length.
[[nodiscard]] constexpr glm::vec3 clampLength(glm::vec3 const& v, float length)
{
    if (glm::length(v) > length) {
        return setLength(v, length);
    }
    return v;
}

} // namespace bg
