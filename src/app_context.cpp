#include "src/app_context.h"

#include "assertr.h"
#include "src/block.h"
#include "src/common.h"
#include "src/lib/json.hpp"
#include "src/logger.h"

#include <SDL3/SDL.h>

#include <fstream>
#include <limits>
#include <random>

namespace bg
{

AppContext::AppContext(SDL_Window* window) : window{ window }
{
    assertr(this->window != nullptr);
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
        std::ifstream blockData{ "assets/data/blocks.json" };
        nlohmann::json const blocks = nlohmann::json::parse(blockData);
        for (auto const& block : blocks["blocks"]) {
            blockRegistry.load(block["name"], block["id"], block["prettyName"], block["breakSounds"],
                               block["hardness"]);
        }
    }

    {
        std::ifstream itemData{ "assets/data/items.json" };
        nlohmann::json const items = nlohmann::json::parse(itemData);
        for (auto const& item : items["items"]) {
            itemRegistry.load(item["id"], item["name"], item["prettyName"]);
        }
    }

    {
        std::ifstream blockItemsData{ "assets/data/blockitems.json" };
        nlohmann::json const blockItems = nlohmann::json::parse(blockItemsData);
        for (auto const& blockItem : blockItems["blockItems"]) {
            blockItemRegistry.load(blockItem["blockName"], blockItem["itemName"]);
        }
        // TODO: Check if the loaded block and item names actually exist.
    }

    std::mt19937_64 gen{ std::random_device{}() };
    std::uniform_int_distribution<std::uint64_t> dist(0, std::numeric_limits<std::uint64_t>::max());
    std::uint64_t const worldSeed = dist(gen);
    logInfo("World Seed: {}", worldSeed);
    game->worldSeed = worldSeed;
}

Dim2d getWindowSize(AppContext& ctx)
{
    Dim2d dim{};
    auto get_window_size_status = SDL_GetWindowSize(ctx.window, &dim.width, &dim.height);
    assertr(get_window_size_status);
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
