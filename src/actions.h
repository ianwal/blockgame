#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <variant>

namespace alf
{

struct SwingAction {
};

struct MoveAction {
    glm::vec3 direction{};
};

using InputAction = std::variant<SwingAction, MoveAction>;

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

} // namespace alf
