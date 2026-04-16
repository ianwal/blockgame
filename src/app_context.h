#pragma once

#include "src/audio.h"
#include "src/block.h"
#include "src/blockitem.h"
#include "src/common.h"
#include "src/crafting.h"
#include "src/game.h"
#include "src/item.h"
#include "src/time.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <cassert>
#include <cstdlib>

namespace bg
{

struct GameOptions {
    float fov{ 105.0f };
};

struct AppContext {
    AppContext(SDL_Window* window);

    SDL_Window* window{};
    std::unique_ptr<GameContext> game{ std::make_unique<GameContext>() };

    AudioResourceManager audioResourceManager{};
    BlockItemRegistry blockItemRegistry{};
    BlockRegistry blockRegistry{};
    ItemRegistry itemRegistry{};
    CraftingRecipeRegistry craftingRecipeRegistry{};

    SDL_AppResult app_quit{ SDL_APP_CONTINUE };
    Timestamp current_time{};
    MIX_Mixer* mixer{};
    MIX_Track* track{};

    GameOptions gameOptions{};

    ~AppContext()
    {
        MIX_DestroyTrack(track);
        MIX_DestroyMixer(mixer);
        SDL_DestroyWindow(window);
    }
};

/// App context singleton.
class App
{
public:
    static AppContext& get();

    static void set(AppContext* newApp);

private:
    static AppContext* app;
};

/// Get current game window size.
Dim2d getWindowSize(AppContext& ctx);

} // namespace bg
