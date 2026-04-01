#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace alf
{

/// @brief Get the camera front vector from the yaw and pitch.
inline glm::vec3 cameraFront(float yaw, float pitch)
{
    glm::vec3 direction{};
    direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    direction.y = glm::sin(glm::radians(pitch));
    direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    return glm::normalize(direction);
}

/// Check if a point is within a 1x1 cube (inclusive).
///
/// The cube position is NOT the center of the cube, it's the world grid position.
constexpr bool isPointInCube(glm::vec3 point, glm::vec3 cubePos)
{
    return point.x >= cubePos.x - 0.5f && point.x <= cubePos.x + 0.5f && point.y >= cubePos.y - 0.5f &&
           point.y <= cubePos.y + 0.5f && point.z >= cubePos.z - 0.5f && point.z <= cubePos.z + 0.5f;
}

struct Camera {
    glm::vec3 cameraPos{ 0.0f, 0.0f, 0.0f };    // Camera position in world space.
    glm::vec3 cameraFront{ 0.0f, 0.0f, -1.0f }; // Camera front in world space.
    glm::vec3 cameraUp{ 0.0f, 1.0f, 0.0f };     // Camera up in world space.

    float yaw{ -90.0 };
    float pitch{ 0.0f };
};

} // namespace alf
