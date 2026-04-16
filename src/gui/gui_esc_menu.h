#pragma once

namespace bg
{

/// Escape menu.
class GuiEscMenu
{
public:
    void open();

    void close();

    void render();

    bool isOpen() const;

private:
    bool isopen{ false };
};

} // namespace bg
