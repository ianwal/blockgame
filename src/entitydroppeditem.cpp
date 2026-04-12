#include "src/entitydroppeditem.h"

#include "src/aabb.h"
#include "src/audio.h"
#include "src/entity.h"
#include "src/game.h"
#include "src/logger.h"
#include "src/utils.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>

namespace bg
{

void EntityDroppedItem::tick()
{
    if (isDead()) {
        return;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Movement
    ////////////////////////////////////////////////////////////////////////////////////////////////
    constexpr float maxVelocity       = 1.0f;
    constexpr float minVelocityToZero = 0.02; // Velocity threshold that is rounded to zero.
    vel                               = clampLength(vel, maxVelocity);
    roundToZero(vel);

    auto const originalVel = vel;
    glm::vec3 newPos{};
    auto aabb = getAABB();
    {
        auto collideCandidates = GlobalGame::get().chunkManager.getNearbyBlocks(pos, 2);
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

    updatePos(newPos);

    // Add gravity
    glm::vec3 const gravityDir = glm::vec3{ 0.0f, -1.0f, 0.0f };
    static_assert(gravityScale > minVelocityToZero);
    vel += gravityDir * gravityScale;

    // Add ground resistance
    if (!isZeroVector(vel)) {
        // Reduce velocity.
        if (isGrounded()) {
            vel = glm::vec3{ 0.0f, vel.y, 0.0f };
        }
    }

    vel = clampLength(vel, maxVelocity);
    roundToZero(vel);

    //////////////////

    // Block Pickup logic
    if ((ticksAlive > numTicksUntilCanPickup) && GlobalGame::get().player.getAABB().intersects(getAABB())) {
        if (GlobalGame::get().playerInventory.addItem(ItemStack{ itemType, 1 }) == 1U) {
            playSound2d("pop");
            setDead();
        }
    }

    if (!isDead()) {
        ++ticksAlive;
    }
}

glm::vec3 EntityDroppedItem::getPos(float partialTick) const
{
    return lerp(prevPos, pos, partialTick);
}

glm::vec3 EntityDroppedItem::getRotation(float partialTick) const
{
    assertr(false);
    return glm::vec3{};
}

EntityType EntityDroppedItem::getEntityType() const
{
    return EntityType::DROPPED_ITEM;
}

bool EntityDroppedItem::isPersistent() const
{
    return true;
}

void EntityDroppedItem::setDead()
{
    dead = true;
}

[[nodiscard]] bool EntityDroppedItem::isDead() const
{
    return dead;
}

////////////////////////////////////////

bool EntityDroppedItem::isGrounded() const
{
    return grounded;
}

glm::vec3 EntityDroppedItem::getVelocity(float partialTick) const
{
    return lerp(prevVel, vel, partialTick);
}

void EntityDroppedItem::setVelocity(glm::vec3 newVel)
{
    vel = newVel;
}

void EntityDroppedItem::updatePos(glm::vec3 newPos)
{
    prevPos = pos;
    pos     = newPos;
}

AABB EntityDroppedItem::getAABB() const
{
    AABB aabb{};
    aabb.minX = pos.x;
    aabb.maxX = pos.x + 0.25f;
    aabb.minZ = pos.z;
    aabb.maxZ = pos.z + 0.25f;
    aabb.minY = pos.y;
    aabb.maxY = pos.y + 0.25f;
    return aabb;
}

void EntityDroppedItem::setItemType(ItemType itemType)
{
    this->itemType = itemType;
}

[[nodiscard]] ItemType EntityDroppedItem::getItemType() const
{
    return itemType;
}

glm::vec3 EntityDroppedItem::getRenderPos(float partialTick) const
{
    return getPos(partialTick) +
           lerp(glm::vec3{ 0, std::sin((static_cast<float>(ticksAlive) - 1.0f) / 10.0f) * 0.1f, 0 },
                glm::vec3{ 0, std::sin(static_cast<float>(ticksAlive) / 10.0f) * 0.1f, 0 }, partialTick);
}

glm::vec3 EntityDroppedItem::getRenderRotation(float partialTick) const
{
    static_cast<void>(partialTick);
    return glm::vec3{ 0, glm::radians(static_cast<float>(ticksAlive % 360U)), 0 };
}

} // namespace bg
