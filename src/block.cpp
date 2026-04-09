#include "src/block.h"

#include "src/app_context.h"
#include "src/logger.h"
#include "src/shaders.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include <array>
#include <cassert>
#include <cstdint>
#include <random>
#include <span>
#include <string>
#include <string_view>

namespace bg
{

GLuint getBlockTexture(BlockType type)
{
    std::string name{};
    switch (type) {
        case BlockType::BLOCK_COBBLE: {
            name = "block.cobble";
        } break;

        case BlockType::BLOCK_LUCKY: {
            name = "block.lucky";
        } break;

        case BlockType::BLOCK_GRASS: {
            name = "block.grass";
        } break;

        case BlockType::BLOCK_DIRT: {
            name = "block.dirt";
        } break;

        case BlockType::BLOCK_AIR:
        default: {
            // Block type texture not added to lookup table. Add it.
            logError("{} failed. Block type has no texture: {}", __func__, static_cast<int>(type));
            assertr(false);
        } break;
    };

    return bg::TextureResourceManagerSingleton::get().get(name);
}

static std::string_view getRandomBlockBreakSoundName(std::span<std::string_view> soundNames)
{
    static std::mt19937 gen{ std::random_device{}() };
    std::uniform_int_distribution<> dist(0, soundNames.size() - 1);
    return soundNames[dist(gen)];
}

std::string getBlockBreakSoundName(BlockType type)
{
    std::string name{};
    switch (type) {
        case BlockType::BLOCK_COBBLE: {
            static auto names =
                std::to_array({ std::string_view{ "block.dig.cobble.1" }, std::string_view{ "block.dig.cobble.2" },

                                std::string_view{ "block.dig.cobble.3" }, std::string_view{ "block.dig.cobble.4" } });
            name = getRandomBlockBreakSoundName(names);
        } break;

        case BlockType::BLOCK_LUCKY: {
            name = "block.dig.cobble.1";
        } break;

        case BlockType::BLOCK_GRASS: {
            static auto names =
                std::to_array({ std::string_view{ "block.dig.grass.1" }, std::string_view{ "block.dig.grass.2" },

                                std::string_view{ "block.dig.grass.3" }, std::string_view{ "block.dig.grass.4" } });
            name = getRandomBlockBreakSoundName(names);
        } break;

        case BlockType::BLOCK_DIRT: {
            static auto names =
                std::to_array({ std::string_view{ "block.dig.dirt.1" }, std::string_view{ "block.dig.dirt.2" },
                                std::string_view{ "block.dig.dirt.3" }, std::string_view{ "block.dig.dirt.4" } });
            name = getRandomBlockBreakSoundName(names);
        } break;

        case BlockType::BLOCK_AIR:
        default: {
            // Block type not added to lookup table. Add it.
            logError("{} failed. Block type has no texture: {}", __func__, static_cast<int>(type));
            assertr(false);
        } break;
    };

    return name;
}

} // namespace bg
