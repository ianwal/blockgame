#include "src/app_context.h"

#include "assertr.h"
#include "src/block.h"
#include "src/common.h"
#include "src/crafting.h"
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

    {
        std::ifstream craftingRecipeData{ "assets/data/craftingrecipes.json" };
        nlohmann::json const craftingRecipes = nlohmann::json::parse(craftingRecipeData);
        for (auto const& craftingRecipe : craftingRecipes["craftingRecipes"]) {
            CraftingRecipe recipe{};
            for (auto const& input : craftingRecipe["input"]) {
                recipe.inputs.push_back(
                    ItemStack{ itemRegistry.getType(input["itemName"]), static_cast<std::uint32_t>(input["amount"]) });
            }
            recipe.output = ItemStack{ itemRegistry.getType(craftingRecipe["output"]["itemName"]),
                                       static_cast<std::uint32_t>(craftingRecipe["output"]["amount"]) };
            craftingRecipeRegistry.load(recipe);
        }
    }

    {
        std::ifstream blockTagsData{ "assets/data/tags/blocks/mineable/pickaxe.json" };
        nlohmann::json const blockTags = nlohmann::json::parse(blockTagsData);
        for (auto const& block : blockTags["mineable.pickaxe"]) {
            blockTagRegistry.addTags(block, std::vector<std::string>{ "mineable.pickaxe" });
        }
        // TODO: Check if the loaded block name actually exists?
    }

    {
        std::ifstream toolTierRequirementsData{ "assets/data/tags/blocks/tool_tier_requirements.json" };
        nlohmann::json const toolTierRequirements = nlohmann::json::parse(toolTierRequirementsData);
        for (auto const& [toolTier, blockNames] : toolTierRequirements.items()) {
            for (auto const& blockName : blockNames) {
                blockTagRegistry.addTags(blockName, std::vector<std::string>{ toolTier });
            }
        }
        // TODO: Check if the loaded block name actually exists?
    }

    {
        std::ifstream blockLootTableData{ "assets/data/block_loot_table.json" };
        nlohmann::json const blockLootTable = nlohmann::json::parse(blockLootTableData);
        for (auto const& [blockName, dropTable] : blockLootTable.items()) {
            blockLootTableRegistry.load(blockName, dropTable["dropItemName"]);
        }
        // TODO: Check if the loaded block name and item name actually exists?
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
