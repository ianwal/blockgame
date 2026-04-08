#pragma once

#include "src/aabb.h"
#include "src/camera.h"

#include <glm/glm.hpp>

namespace bg
{

class Player
{
public:
    Player();

    Camera fpCamera{ glm::vec3{ 0.f, 0.f, 0.f } }; // First person camera

    /// Current viewport camera. Will never be nullptr. Use this for rendering.
    Camera* currentCamera = &fpCamera;

    void tick();

    [[nodiscard]] glm::vec3 getPos(float partialTick) const;

    void setVelocity(glm::vec3 newVel);

    [[nodiscard]] glm::vec3 getVelocity() const;

    void updatePos(glm::vec3 newPos);

    [[nodiscard]] float getMoveSpeedMultiplier() const;

    void setMoveSpeedMultiplier(float moveSpeedMultiplier);

    [[nodiscard]] bool isGrounded() const;

    void setFlying(bool enabled);

    [[nodiscard]] bool isFlying() const;

    void setNoclip(bool enabled);

    [[nodiscard]] bool isNoclip() const;

private:
    [[nodiscard]] AABB getAABB() const;

    glm::vec3 pos{};         ///< Position.
    glm::vec3 prevPos = pos; ///< Position on the previous tick.
    glm::vec3 vel{};         ///< Velocity.

    float moveSpeedMultiplier{ 1.0 };

    bool noclip{ false };
    bool flying{ false };

    bool grounded{ false };

    static constexpr float gravityScale{ 0.04 };
};

} // namespace bg
