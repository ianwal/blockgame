#pragma once

#include "src/common.h"
#include "src/game.h"
#include "src/time.h"

#include <SDL3/SDL.h>

#include <cassert>
#include <cstdlib>

namespace alf
{

struct AppContext {
    AppContext(SDL_Window* window) : window{ window }
    {
        assert(this->window != nullptr);
    }

    SDL_Window* window{};
    std::unique_ptr<GameContext> game{ std::make_unique<GameContext>() };

    SDL_AppResult app_quit{ SDL_APP_CONTINUE };
    Timestamp current_time{};

    ~AppContext()
    {
        SDL_DestroyWindow(window);
    }
};

/// Get current game window size.
Dim2d getWindowSize(AppContext& ctx);

} // namespace alf
