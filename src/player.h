#pragma once

#include <glm/glm.hpp>

namespace alf
{

class Player
{
public:
    Player() = default;

    Camera fpCamera{}; // First person camera

    /// Current viewport camera. Will never be nullptr. Use this for rendering.
    Camera* currentCamera = &fpCamera;

    float moveSpeedMultiplier{ 1.0 };

    glm::vec3 pos{};
    glm::vec3 vel{};
};

} // namespace alf
