#include "src/player.h"

#include "src/logger.h"
#include "src/utils.h"

#include <glm/glm.hpp>

#include <algorithm>

namespace bg
{

void Player::tick()
{
    constexpr float maxVelocity       = 1.0f;
    constexpr float minVelocityToZero = 0.02; // Velocity threshold that is rounded to zero.
    vel                               = clampLength(vel, maxVelocity);

    auto newPos = pos + vel; // time is already factored in because it's constant between ticks.
    updatePos(newPos);

    if (noclip) {

        // Air resistance
        if (!isZeroVector(vel)) {

            // Get the direction player is moving towards currently.
            glm::vec3 velDir = glm::normalize(vel);

            // Reduce velocity a bit.
            vel -= (velDir * 0.1f);
        }

    } else {
        // Add gravity
        glm::vec3 const gravityDir = glm::vec3{ 0.0f, -1.0f, 0.0f };
        constexpr float gravityScale{ 0.5 };
        static_assert(gravityScale > minVelocityToZero);
        vel += gravityDir * gravityScale;
    }

    // TODO: Check collision

    vel = clampLength(vel, maxVelocity);

    // Round velocity to zero if near zero.
    if (!isZeroVector(vel)) {
        if (glm::length(vel) < 0.02) {
            vel = glm::vec3{ 0.0f, 0.0f, 0.0f };
        }
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

void Player::setVelocity(glm::vec3 newVel)
{
    vel = newVel;
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
