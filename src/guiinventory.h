#pragma once

#include "src/playerinventory.h"

namespace bg
{

class GuiInventory
{
public:
    void render(PlayerInventory const& playerInventory);
};

} // namespace bg
