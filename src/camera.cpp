#include "src/camera.h"

#include <glm/glm.hpp>

namespace bg
{

glm::vec3 cameraFront(float yaw, float pitch)
{
    glm::vec3 direction{};
    direction.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    direction.y = glm::sin(glm::radians(pitch));
    direction.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
    return glm::normalize(direction);
}

Camera::Camera(glm::vec3 pos)
{
    prevCameraPos = pos;
    cameraPos     = pos;
}

void Camera::updatePos(glm::vec3 newPos)
{
    prevCameraPos = cameraPos;
    cameraPos     = newPos;
}

glm::vec3 Camera::getPos(float partialTick) const
{
    return lerp(prevCameraPos, cameraPos, partialTick);
}

void Camera::updateFront(glm::vec3 front)
{
    this->prevCameraFront = this->cameraFront;
    this->cameraFront     = front;
}

glm::vec3 Camera::getFront(float partialTick) const
{
    return lerp(prevCameraFront, cameraFront, partialTick);
}

glm::vec3 Camera::getUp() const
{
    return cameraUp;
}

void Camera::setYaw(float yaw)
{
    this->yaw = yaw;
}

float Camera::getYaw() const
{
    return yaw;
}

void Camera::setPitch(float pitch)
{
    this->pitch = pitch;
}

float Camera::getPitch() const
{
    return this->pitch;
}

} // namespace bg
