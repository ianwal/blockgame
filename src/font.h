#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cassert>

namespace Alf
{

// CALL ONCE AT START OF PROGRAM.
bool global_font_init();

// CALL ONCE AT END OF PROGRAM.
void destroy_global_font();

TTF_Font &get_font();

struct Font
{
    Font() : font{&get_font()}
    {
    }

    TTF_Font *font{};
};

} // namespace Alf
