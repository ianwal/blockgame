#pragma once

#include "src/camera.h"

#include <glm/glm.hpp>

namespace bg
{

class Player
{
public:
    Player() = default;

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
    glm::vec3 pos{};         ///< Position.
    glm::vec3 prevPos = pos; ///< Position on the previous tick.
    glm::vec3 vel{};         ///< Velocity.

    float moveSpeedMultiplier{ 1.0 };

    bool noclip{ true };
};

} // namespace bg
