#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <variant>

namespace bg
{

struct SwingAction {
};

struct MoveAction {
    enum class Direction : std::uint8_t {
        LEFT = 0,
        RIGHT,
        UP,
        DOWN,
        FORWARD,
        BACKWARD,
    };

    Direction direction{};
};

using InputAction = std::variant<SwingAction, MoveAction>;

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

} // namespace bg
