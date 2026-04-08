#include "src/player.h"

#include "src/game.h"
#include "src/logger.h"
#include "src/utils.h"

#include <glm/glm.hpp>

#include <algorithm>

namespace bg
{

namespace
{

AABB getBlockAABB(Block const& block)
{
    AABB aabb{};
    aabb.minX = block.pos.x;
    aabb.maxX = block.pos.x + 1.0f;
    aabb.minY = block.pos.y;
    aabb.maxY = block.pos.y + 1.0f; // todo: is minus?
    aabb.minZ = block.pos.z;
    aabb.maxZ = block.pos.z + 1.0f;
    return aabb;
}

} // namespace

Player::Player()
{
    // Ensure the fp camera position is offset correctly from the player.
    updatePos(pos);
}

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

        auto collideCandidates = GlobalGame::get().chunkManager.getNearbyBlocks(pos, 2);

        // Hulk mode (for debugging)
        auto aabb = getAABB();
        for (auto block : collideCandidates) {
            if (block->type == BlockType::BLOCK_AIR) {
                continue;
            }
            if (getAABB().intersects(getBlockAABB(*block))) {
                logInfo("Player pos: X: {} Y: {} Z: {}", pos.x, pos.y, pos.z);
                logInfo("Player AABB: min X: {} min Y: {} min Z: {}", aabb.minX, aabb.minY, aabb.minZ);
                logInfo("Collided with block: X: {} Y: {} Z: {}", block->pos.x, block->pos.y, block->pos.z);
                GlobalGame::get().levelManager.removeBlock(block);
            }
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
    fpCamera.updatePos(newPos - glm::vec3{ 0, 0.2, 0 }); // First person camera is centered near the head of the player.
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
    float const playerWidth  = 0.6;
    float const playerHeight = 1.8;
    // TODO: Is this / 2 correct or should the min be the pos min and max is + the width?
    aabb.minX = pos.x;
    aabb.maxX = pos.x + playerWidth;
    aabb.minZ = pos.z;
    aabb.maxZ = pos.z + playerWidth;
    // The player position is at the bottom of the player.
    aabb.minY = pos.y - playerHeight;
    aabb.maxY = pos.y;
    return aabb;
}

} // namespace bg
