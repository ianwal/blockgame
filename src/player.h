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

    [[nodiscard]] glm::vec3 getVelocity(float partialTick) const;

    void updatePos(glm::vec3 newPos);

    [[nodiscard]] float getMoveSpeedMultiplier() const;

    void setMoveSpeedMultiplier(float moveSpeedMultiplier);

    [[nodiscard]] bool isGrounded() const;

    void setSneaking(bool enabled);

    [[nodiscard]] bool isSneaking() const;

    void setFlying(bool enabled);

    [[nodiscard]] bool isFlying() const;

    void setNoclip(bool enabled);

    [[nodiscard]] bool isNoclip() const;

    void swing();

    float getSwingProgress(float partialTick);

    [[nodiscard]] AABB getAABB() const;

private:
    glm::vec3 pos{};         ///< Position.
    glm::vec3 prevPos = pos; ///< Position on the previous tick.
    glm::vec3 vel{};         ///< Velocity.
    glm::vec3 prevVel = vel; ///< Velocity on the previous tick.

    float moveSpeedMultiplier{ 1.0 };

    bool noclip{ false };
    bool flying{ false };

    bool grounded{ false };

    bool swinging{ false };

    bool sneaking{ false };

    std::uint32_t prevSwingProgressTicks{ 0 };
    std::uint32_t swingProgressTicks{ 0 };
    static constexpr std::uint32_t swingDurationTicks{ 6 };

    std::uint32_t footstepProgressTicks{ 0 };
    static constexpr std::uint32_t footstepDurationTicks{ 12 };

    static constexpr float gravityScale{ 0.04 };
};

} // namespace bg
