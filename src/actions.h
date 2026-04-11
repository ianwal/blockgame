#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <variant>

namespace bg
{

struct SwingAction {
};

struct MoveAction {
    enum Direction : std::uint8_t {
        NONE     = 0,
        LEFT     = 1,
        RIGHT    = 1 << 1,
        UP       = 1 << 2,
        DOWN     = 1 << 3,
        FORWARD  = 1 << 4,
        BACKWARD = 1 << 5,
    };

    Direction direction{};
};

struct SneakAction {
};

struct PlaceAction {
};

struct DropItemAction {
    // TODO: Add entire stack bool for CTRL+Q
};

struct ChangeHotbarPosAction {
    std::uint32_t index{};
};

using InputAction =
    std::variant<SwingAction, MoveAction, SneakAction, PlaceAction, DropItemAction, ChangeHotbarPosAction>;

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

} // namespace bg
