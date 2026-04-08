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

constexpr float playerWidth  = 0.6;
constexpr float playerHeight = 1.8;

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

    auto const originalVel = vel;
    glm::vec3 newPos{};
    if (noclip) {
        newPos = pos + vel;
    } else {
        auto aabb = getAABB();
        {
            auto collideCandidates = GlobalGame::get().chunkManager.getNearbyBlocks(pos, 30);
            for (auto block : collideCandidates) {
                if (block->type == BlockType::BLOCK_AIR) {
                    continue;
                }

                vel.y = aabb.getNearestClipY(getBlockAABB(*block), vel.y);
            }
            aabb.translate(glm::vec3{ 0.0f, vel.y, 0.0f });

            for (auto block : collideCandidates) {
                if (block->type == BlockType::BLOCK_AIR) {
                    continue;
                }

                vel.x = aabb.getNearestClipX(getBlockAABB(*block), vel.x);
            }
            aabb.translate(glm::vec3{ vel.x, 0.0f, 0.0f });

            for (auto block : collideCandidates) {
                if (block->type == BlockType::BLOCK_AIR) {
                    continue;
                }

                vel.z = aabb.getNearestClipZ(getBlockAABB(*block), vel.z);
            }
            aabb.translate(glm::vec3{ 0.0f, 0.0f, vel.z });
        }

        if (vel.x != originalVel.x) {
            vel.x = 0;
        }

        if (vel.y != originalVel.y) {
            vel.y = 0;
        }

        if (vel.z != originalVel.z) {
            vel.z = 0;
        }

        // Note: These are the min X and Z and max Y based on how the player AABB is defined (see getAABB()).
        newPos = glm::vec3{ aabb.minX, aabb.minY, aabb.minZ };
    }

    updatePos(newPos);

    if (flying) {
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
    fpCamera.updatePos(newPos -
                       glm::vec3{ 0.1, -1, 0.1 }); // First person camera is centered near the head of the player.
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

} // namespace bg
