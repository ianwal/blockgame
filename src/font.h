#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cassert>

namespace Alf
{

TTF_Font &get_font();

struct Font
{
    Font() : font{&get_font()}
    {
    }

    TTF_Font *font{};
};

} // namespace Alf
