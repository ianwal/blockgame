#pragma once

#include "src/utils.h"

#include <glm/glm.hpp>

namespace bg
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

class Camera
{
public:
    Camera(glm::vec3 pos)
    {
        prevCameraPos = pos;
        cameraPos     = pos;
    }

    void updatePos(glm::vec3 newPos)
    {
        prevCameraPos = cameraPos;
        cameraPos     = newPos;
    }

    // Get the interpolated camera position for the current partial tick.
    glm::vec3 getPos(float partialTick) const
    {
        return lerp(prevCameraPos, cameraPos, partialTick);
    }

    void updateFront(glm::vec3 front)
    {
        this->prevCameraFront = this->cameraFront;
        this->cameraFront     = front;
    }

    // Get the interpolated camera front for the current partial tick.
    glm::vec3 getFront(float partialTick) const
    {
        return lerp(prevCameraFront, cameraFront, partialTick);
    }

    glm::vec3 getUp() const
    {
        return cameraUp;
    }

    void setYaw(float yaw)
    {
        this->yaw = yaw;
    }

    float getYaw() const
    {
        return yaw;
    }

    void setPitch(float pitch)
    {
        this->pitch = pitch;
    }

    float getPitch() const
    {
        return this->pitch;
    }

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
