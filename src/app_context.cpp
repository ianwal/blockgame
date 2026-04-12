#include "src/app_context.h"

#include "src/common.h"
#include "src/lib/json.hpp"

#include <SDL3/SDL.h>

#include <cassert>
#include <fstream>

namespace bg
{

AppContext::AppContext(SDL_Window* window) : window{ window }
{
    assert(this->window != nullptr);
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    track = MIX_CreateTrack(mixer);

    std::ifstream soundFiles{ "assets/data/sounds.json" };
    nlohmann::json const soundData = nlohmann::json::parse(soundFiles);
    for (auto const& sound : soundData["sounds"]) {
        audioResourceManager.load(sound["name"], sound["path"]);
    }
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
