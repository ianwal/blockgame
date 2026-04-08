#pragma once

#include "src/camera.h"

#include <glm/glm.hpp>

namespace bg
{

struct AABB {
    float minX{};
    float maxX{};
    float minY{};
    float maxY{};
    float minZ{};
    float maxZ{};

    [[nodiscard]] constexpr bool intersectsX(AABB const& rhs) const
    {
        return ((minX < rhs.maxX) && (maxX > rhs.minX));
    }

    [[nodiscard]] constexpr bool intersectsY(AABB const& rhs) const
    {
        return ((minY < rhs.maxY) && (maxY > rhs.minY));
    }

    [[nodiscard]] constexpr bool intersectsZ(AABB const& rhs) const
    {
        return ((minZ < rhs.maxZ) && (maxZ > rhs.minZ));
    }

    [[nodiscard]] constexpr bool intersects(AABB const& rhs) const
    {
        return intersectsX(rhs) && intersectsY(rhs) && intersectsZ(rhs);
    }

    /// Get the nearest clipping position for a moving AABB that may be moving into/past the other AABB.
    [[nodiscard]] constexpr float getNearestClipX(AABB const& rhs, float deltaX) const
    {
        // If it doesn't intersect the other two axes, then it doesn't need to be moved.
        if (!(intersectsY(rhs) && intersectsZ(rhs))) {
            return deltaX;
        }

        // if moving down and
        if (deltaX < 0 && minX >= rhs.maxX) {
            float const clippedDelta = rhs.maxX - minX;
            if (deltaX < clippedDelta) {
                return clippedDelta;
            }
        }

        if (deltaX > 0 && maxX <= rhs.minX) {
            float const clippedDelta = rhs.minX - maxX;
            if (deltaX > clippedDelta) {
                return clippedDelta;
            }
        }
        return deltaX;
    }

    /// Get the nearest clipping position for a moving AABB that may be moving into/past the other AABB.
    [[nodiscard]] constexpr float getNearestClipY(AABB const& rhs, float deltaY) const
    {
        // If it doesn't intersect the other two axes, then it doesn't need to be moved.
        if (!(intersectsX(rhs) && intersectsZ(rhs))) {
            return deltaY;
        }

        // if moving down and
        if (deltaY < 0 && minY >= rhs.maxY) {
            float const clippedDelta = rhs.maxY - minY;
            if (deltaY < clippedDelta) {
                return clippedDelta;
            }
        }

        if (deltaY > 0 && maxY <= rhs.minY) {
            float const clippedDelta = rhs.minY - maxY;
            if (deltaY > clippedDelta) {
                return clippedDelta;
            }
        }
        return deltaY;
    }

    /// Get the nearest clipping position for a moving AABB that may be moving into/past the other AABB.
    [[nodiscard]] constexpr float getNearestClipZ(AABB const& rhs, float deltaZ) const
    {
        // If it doesn't intersect the other two axes, then it doesn't need to be moved.
        if (!(intersectsX(rhs) && intersectsY(rhs))) {
            return deltaZ;
        }

        // if moving down and
        if (deltaZ < 0 && minZ >= rhs.maxZ) {
            float const clippedDelta = rhs.maxZ - minZ;
            if (deltaZ < clippedDelta) {
                return clippedDelta;
            }
        }

        if (deltaZ > 0 && maxZ <= rhs.minZ) {
            float const clippedDelta = rhs.minZ - maxZ;
            if (deltaZ > clippedDelta) {
                return clippedDelta;
            }
        }
        return deltaZ;
    }

    constexpr void translate(glm::vec3 offset)
    {
        minX += offset.x;
        maxX += offset.x;
        minY += offset.y;
        maxY += offset.y;
        minZ += offset.z;
        maxZ += offset.z;
    }
};

class Player
{
public:
    Player();

    Camera fpCamera{ glm::vec3{ 0.f, 0.f, 0.f } }; // First person camera

    /// Current viewport camera. Will never be nullptr. Use this for rendering.
    Camera* currentCamera = &fpCamera;

    void tick();

    glm::vec3 getPos(float partialTick) const;

    void setVelocity(glm::vec3 newVel);

    glm::vec3 getVelocity() const;

    void updatePos(glm::vec3 newPos);

    float getMoveSpeedMultiplier() const;

    void setMoveSpeedMultiplier(float moveSpeedMultiplier);

private:
    AABB getAABB() const;

    glm::vec3 pos{};         ///< Position.
    glm::vec3 prevPos = pos; ///< Position on the previous tick.
    glm::vec3 vel{};         ///< Velocity.

    float moveSpeedMultiplier{ 1.0 };

    bool noclip{ false };
    bool flying{ true };
};

} // namespace bg
