#pragma once

#include "src/utils.h"

#include <glm/glm.hpp>

namespace bg
{

/// @brief Get the camera front vector from the yaw and pitch.
glm::vec3 cameraFront(float yaw, float pitch);

/// Check if a point is within a 1x1 cube (inclusive).
///
/// The cube position is NOT the center of the cube, it's the world grid position.
constexpr bool isPointInCube(glm::vec3 point, glm::vec3 cubePos)
{
    return point.x >= cubePos.x - 0.5f && point.x <= cubePos.x + 0.5f && point.y >= cubePos.y - 0.5f &&
           point.y <= cubePos.y + 0.5f && point.z >= cubePos.z - 0.5f && point.z <= cubePos.z + 0.5f;
}

class Camera
{
public:
    Camera(glm::vec3 pos);

    void updatePos(glm::vec3 newPos);

    // Get the interpolated camera position for the current partial tick.
    glm::vec3 getPos(float partialTick) const;

    void updateFront(glm::vec3 front);

    // Get the interpolated camera front for the current partial tick.
    glm::vec3 getFront(float partialTick) const;

    glm::vec3 getUp() const;

    void setYaw(float yaw);

    float getYaw() const;

    void setPitch(float pitch);

    float getPitch() const;

private:
    glm::vec3 cameraPos{ 0.0f, 0.0f, 0.0f };    // Camera position in world space.
    glm::vec3 cameraFront{ 0.0f, 0.0f, -1.0f }; // Camera front in world space.
    glm::vec3 cameraUp{ 0.0f, 1.0f, 0.0f };     // Camera up in world space.

    glm::vec3 prevCameraPos{ 0.0f, 0.0f, 0.0f };    // Previous camera position in world space.
    glm::vec3 prevCameraFront{ 0.0f, 0.0f, -1.0f }; // Previous camera front in world space.

    float yaw{ -90.0 };
    float pitch{ 0.0f };
};

} // namespace bg
