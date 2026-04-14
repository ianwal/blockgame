#include "src/player.h"

#include "src/aabb.h"
#include "src/audio.h"
#include "src/game.h"
#include "src/logger.h"
#include "src/utils.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>

namespace bg
{

namespace
{

constexpr float playerWidth  = 0.6;
constexpr float playerHeight = 1.8;

} // namespace

Player::Player()
{
    // Ensure the fp camera position is offset correctly from the player.
    updatePos(pos);
}

void Player::tick()
{
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Movement
    ////////////////////////////////////////////////////////////////////////////////////////////////
    constexpr float maxVelocity       = 1.0f;
    constexpr float minVelocityToZero = 0.02; // Velocity threshold that is rounded to zero.
    vel                               = clampLength(vel, maxVelocity);
    roundToZero(vel);

    auto const originalVel = vel;
    glm::vec3 newPos{};
    if (noclip) {
        newPos = pos + vel;
    } else {
        auto aabb = getAABB();
        {
            auto collideCandidates = GlobalGame::get().chunkManager.getNearbyBlocks(pos, 30);
            for (auto const& block : collideCandidates) {
                if (block.type == BlockType::BLOCK_AIR) {
                    continue;
                }

                vel.y = aabb.getNearestClipY(getBlockAABB(block), vel.y);
            }
            aabb.translate(glm::vec3{ 0.0f, vel.y, 0.0f });

            for (auto const& block : collideCandidates) {
                if (block.type == BlockType::BLOCK_AIR) {
                    continue;
                }

                vel.x = aabb.getNearestClipX(getBlockAABB(block), vel.x);
            }
            aabb.translate(glm::vec3{ vel.x, 0.0f, 0.0f });

            for (auto const& block : collideCandidates) {
                if (block.type == BlockType::BLOCK_AIR) {
                    continue;
                }

                vel.z = aabb.getNearestClipZ(getBlockAABB(block), vel.z);
            }
            aabb.translate(glm::vec3{ 0.0f, 0.0f, vel.z });
        }

        if (vel.x != originalVel.x) {
            vel.x = 0;
        }

        if (!grounded) {
            footstepProgressTicks = footstepDurationTicks - 1;
        }

        grounded = false; // reset before possibly setting below
        if (vel.y != originalVel.y) {
            vel.y = 0;
            if (originalVel.y < 0.0) {
                grounded = true;
            }
        }

        if (vel.z != originalVel.z) {
            vel.z = 0;
        }

        // Round velocity to zero if near zero.

        // Note: These are the min X and Z and max Y based on how the player AABB is defined (see getAABB()).
        newPos = glm::vec3{ aabb.minX, aabb.minY, aabb.minZ };
        roundToZero(newPos);
    }

    updatePos(newPos);

    if (flying) {
        grounded = false; // Never grounded while flying is enabled.

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
        static_assert(gravityScale > minVelocityToZero);
        vel += gravityDir * gravityScale;

        // Footsteps
        if (!isZeroVector(vel)) {
            if (isGrounded() && ((std::abs(vel.x) > 0.0f) || (std::abs(vel.z) > 0.0f))) {
                ++footstepProgressTicks;
                if (footstepProgressTicks >= footstepDurationTicks) {
                    auto blockStandingOnResult = GlobalGame::get().chunkManager.getBlock(
                        ivec3{ std::floor(pos.x), std::floor(pos.y) - 1, std::floor(pos.z) });
                    if (blockStandingOnResult.has_value() &&
                        blockStandingOnResult.value().type != BlockType::BLOCK_AIR) {
                        playSound2d(getBlockBreakSoundName(blockStandingOnResult.value().type));
                    }
                    footstepProgressTicks = 0;
                }
            }
        }

        // Add ground resistance
        if (!isZeroVector(vel)) {
            // Reduce velocity.
            if (isGrounded()) {
                vel = glm::vec3{ 0.0f, vel.y, 0.0f };
            }
        }

        // Air resistance
        if (!isZeroVector(vel)) {
            // Get the direction player is moving towards currently.
            glm::vec3 velDir = glm::normalize(vel);

            // Reduce velocity a bit.
            vel -= (glm::vec3{ 1, 0, 1 } * (velDir * 0.1f));
        }
    }

    vel = clampLength(vel, maxVelocity);
    roundToZero(vel);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Attack
    ////////////////////////////////////////////////////////////////////////////////////////////////
    prevSwingProgressTicks = swingProgressTicks;
    if (swinging) {
        ++swingProgressTicks;
        if (swingProgressTicks >= swingDurationTicks) {
            swinging           = false;
            swingProgressTicks = 0;
        }
    }
    prevVel = vel;
}

glm::vec3 Player::getPos(float partialTick) const
{
    return lerp(prevPos, pos, partialTick);
}

glm::vec3 Player::getVelocity() const
{
    return vel;
}

glm::vec3 Player::getVelocity(float partialTick) const
{
    return lerp(prevVel, vel, partialTick);
}

void Player::setVelocity(glm::vec3 newVel)
{
    vel = newVel;
}

void Player::updatePos(glm::vec3 newPos)
{
    prevPos = pos;
    pos     = newPos;
    glm::vec3 cameraOffset{ 0.1, -1, 0.1 };
    if (sneaking) {
        cameraOffset += glm::vec3{ 0.0f, 0.2, 0.0f };
    }
    fpCamera.updatePos(newPos - cameraOffset); // First person camera is centered near the head of the player.
}

float Player::getMoveSpeedMultiplier() const
{
    return moveSpeedMultiplier;
}

void Player::setMoveSpeedMultiplier(float moveSpeedMultiplier)
{
    this->moveSpeedMultiplier = moveSpeedMultiplier;
}

AABB Player::getAABB() const
{
    // Note: Player rotation doesn't affect AABB.
    AABB aabb{};
    aabb.minX = pos.x;
    aabb.maxX = pos.x + playerWidth;
    aabb.minZ = pos.z;
    aabb.maxZ = pos.z + playerWidth;
    aabb.minY = pos.y;
    aabb.maxY = pos.y + playerHeight;
    return aabb;
}

bool Player::isGrounded() const
{
    return grounded;
}

void Player::setSneaking(bool enabled)
{
    sneaking = enabled;
}

bool Player::isSneaking() const
{
    return sneaking;
}

void Player::setFlying(bool enabled)
{
    flying = enabled;
}

bool Player::isFlying() const
{
    return flying;
}

void Player::setNoclip(bool enabled)
{
    noclip = enabled;
}

bool Player::isNoclip() const
{
    return noclip;
}

void Player::swing()
{
    swinging = true;
}

float Player::getSwingProgress(float partialTick)
{
    float v = (((float) swingProgressTicks) / ((float) swingDurationTicks)) -
              (((float) prevSwingProgressTicks) / ((float) swingDurationTicks));
    if (v < 0.0f) {
        v += 1.0f;
    }
    return (((float) prevSwingProgressTicks) / ((float) swingDurationTicks)) + v * partialTick;
}

} // namespace bg
