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
    AppContext(SDL_Renderer* renderer, SDL_Window* window) : renderer{ renderer }, window{ window }
    {
        assert(this->renderer != nullptr);
        assert(this->window != nullptr);
    }

    SDL_Renderer* renderer{};
    SDL_Window* window{};
    std::unique_ptr<GameContext> game{ std::make_unique<GameContext>() };

    SDL_AppResult app_quit{ SDL_APP_CONTINUE };
    Timestamp current_time{};

    ~AppContext()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }
};

/// Get current game window size.
Dim2d getWindowSize(AppContext& ctx);

} // namespace alf
