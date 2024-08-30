#pragma once

#include "src/button.h"
#include <cstdint>
#include <deque>
#include <variant>

namespace Alf
{

using Render_layer_t = uint8_t;

using Gui_object_var_t = std::variant<Alf::VisibleButton, Alf::TextButton, Alf::Text>;

struct GuiObjects
{
    void add(Gui_object_var_t &&obj)
    {
        objects.push_back(obj);
    }

    void render(SDL_Renderer &renderer)
    {
        for (auto &obj : this->objects)
        {
            std::visit([&renderer](auto &&arg) { arg.render(renderer); }, obj);
        }
    }

    std::deque<Gui_object_var_t> objects{};
};

struct GameContext
{
    GuiObjects gui_objects{};
};

} // namespace Alf
