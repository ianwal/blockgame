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
        audioResourceManager.load("block.dig.grass.1", "assets/audio/sounds/Grass_dig4.ogg"),
            audioResourceManager.load("block.dig.dirt.1", "assets/audio/sounds/Gravel_dig4.ogg"),
            audioResourceManager.load("block.dig.cobble.1", "assets/audio/sounds/Stone_dig1shifted.ogg");
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
