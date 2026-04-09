#pragma once

#include "src/audio.h"
#include "src/common.h"
#include "src/game.h"
#include "src/time.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <cassert>
#include <cstdlib>

namespace bg
{

struct AppContext {
    AppContext(SDL_Window* window) : window{ window }
    {
        assert(this->window != nullptr);
        mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        track = MIX_CreateTrack(mixer);
        audioResourceManager.load("block.dig.cobble.1", "assets/audio/sounds/block/stone/break/stone1.ogg")
            .load("block.dig.cobble.2", "assets/audio/sounds/block/stone/break/stone2.ogg")
            .load("block.dig.cobble.3", "assets/audio/sounds/block/stone/break/stone3.ogg")
            .load("block.dig.cobble.4", "assets/audio/sounds/block/stone/break/stone4.ogg")
            .load("block.dig.grass.1", "assets/audio/sounds/block/grass/break/grass1.ogg")
            .load("block.dig.grass.2", "assets/audio/sounds/block/grass/break/grass2.ogg")
            .load("block.dig.grass.3", "assets/audio/sounds/block/grass/break/grass3.ogg")
            .load("block.dig.grass.4", "assets/audio/sounds/block/grass/break/grass4.ogg")
            .load("block.dig.dirt.1", "assets/audio/sounds/block/dirt/break/dirt1.ogg")
            .load("block.dig.dirt.2", "assets/audio/sounds/block/dirt/break/dirt2.ogg")
            .load("block.dig.dirt.3", "assets/audio/sounds/block/dirt/break/dirt3.ogg")
            .load("block.dig.dirt.4", "assets/audio/sounds/block/dirt/break/dirt4.ogg");
    }

    SDL_Window* window{};
    std::unique_ptr<GameContext> game{ std::make_unique<GameContext>() };

    AudioResourceManager audioResourceManager{};

    SDL_AppResult app_quit{ SDL_APP_CONTINUE };
    Timestamp current_time{};
    MIX_Mixer* mixer{};
    MIX_Track* track{};

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
