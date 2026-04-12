#include "src/app_context.h"

#include "src/common.h"

#include <SDL3/SDL.h>

#include <cassert>

namespace bg
{

AppContext::AppContext(SDL_Window* window) : window{ window }
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
        .load("block.dig.dirt.4", "assets/audio/sounds/block/dirt/break/dirt4.ogg")
        .load("pop", "assets/audio/sounds/misc/pop.opus");
}

Dim2d getWindowSize(AppContext& ctx)
{
    Dim2d dim{};
    auto get_window_size_status = SDL_GetWindowSize(ctx.window, &dim.width, &dim.height);
    assert(get_window_size_status);
    return dim;
}

AppContext* App::app{};

AppContext& App::get()
{
    return *App::app;
}

void App::set(AppContext* newApp)
{
    App::app = newApp;
}

} // namespace bg
