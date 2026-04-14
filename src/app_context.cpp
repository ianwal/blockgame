#include "src/app_context.h"

#include "src/block.h"
#include "src/common.h"
#include "src/lib/json.hpp"
#include "src/logger.h"

#include <SDL3/SDL.h>

#include <cassert>
#include <fstream>
#include <random>

namespace bg
{

AppContext::AppContext(SDL_Window* window) : window{ window }
{
    assert(this->window != nullptr);
    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    track = MIX_CreateTrack(mixer);

    {
        std::ifstream soundFiles{ "assets/data/sounds.json" };
        nlohmann::json const soundData = nlohmann::json::parse(soundFiles);
        for (auto const& sound : soundData["sounds"]) {
            audioResourceManager.load(sound["name"], sound["path"]);
        }
    }

    {
        std::ifstream blockFiles{ "assets/data/blocks.json" };
        nlohmann::json const blockData = nlohmann::json::parse(blockFiles);
        for (auto const& block : blockData["blocks"]) {
            blockManager.load(block["name"], block["id"], block["prettyName"], block["breakSounds"]);
        }
    }

    std::mt19937 gen{ std::random_device{}() };
    std::uniform_int_distribution<> dist(0, 10000000);
    std::uint64_t const worldSeed = dist(gen);
    logInfo("World Seed: {}", worldSeed);
    game->worldSeed = worldSeed;
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
