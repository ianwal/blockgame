#include "src/player.h"

#include "src/utils.h"

#include <glm/glm.hpp>

#include <algorithm>

namespace bg
{

void Player::tick()
{
    if (noclip) {
    } else {
        // TODO: Check collision

        // Gravity
        auto newPos = pos + vel;
        updatePos(newPos);

        // Compute new velocity from gravity.
        constexpr float maxVelocity = 1.0f; // Max velocity per component of the velocity vector.
        vel += glm::vec3{ 0.0, -0.02, 0.0 };
        vel = glm::vec3{ std::clamp(vel.x, -maxVelocity, maxVelocity), std::clamp(vel.y, -maxVelocity, maxVelocity),
                         std::clamp(vel.z, -maxVelocity, maxVelocity) };
    }
}

glm::vec3 Player::getPos(float partialTick) const
{
    return lerp(prevPos, pos, partialTick);
}

glm::vec3 Player::getVelocity() const
{
    return vel;
}

void Player::updatePos(glm::vec3 newPos)
{
    prevPos = pos;
    pos     = newPos;
    fpCamera.updatePos(newPos);
}

float Player::getMoveSpeedMultiplier() const
{
    return moveSpeedMultiplier;
}

void Player::setMoveSpeedMultiplier(float moveSpeedMultiplier)
{
    this->moveSpeedMultiplier = moveSpeedMultiplier;
}

} // namespace bg
