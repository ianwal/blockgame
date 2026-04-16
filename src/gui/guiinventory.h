#pragma once

#include "src/playerinventory.h"

namespace bg
{

class GuiInventory
{
public:
    void open();

    void close();

    void render(PlayerInventory const& playerInventory);

    bool isOpen() const;

private:
    bool isopen{ false };
};

} // namespace bg
