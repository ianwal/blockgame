#pragma once

#include "src/playerinventory.h"

namespace bg
{

class GuiHotbar
{
public:
    void render(PlayerInventory const& playerInventory);
};

} // namespace bg
