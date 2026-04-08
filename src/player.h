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

    constexpr bool intersects(AABB const& rhs) const
    {
        return ((minX < rhs.maxX) && (maxX > rhs.minX)) && ((minY < rhs.maxY) && (maxY > rhs.minY)) &&
               ((minZ < rhs.maxZ) && (maxZ > rhs.minZ));
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

    bool noclip{ true };
};

} // namespace bg
