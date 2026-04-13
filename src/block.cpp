#include "src/block.h"

#include "src/aabb.h"
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

AABB getBlockAABB(Block const& block)
{
    AABB aabb{};
    aabb.minX = block.pos.x;
    aabb.maxX = block.pos.x + 1.0f;
    aabb.minY = block.pos.y;
    aabb.maxY = block.pos.y + 1.0f; // todo: is minus?
    aabb.minZ = block.pos.z;
    aabb.maxZ = block.pos.z + 1.0f;
    return aabb;
}

GLuint getBlockTexture(BlockType type)
{
    std::string name{};
    switch (type) {
        case BlockType::BLOCK_STONE: {
            name = "block.stone";
        } break;

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

        case BlockType::BLOCK_OAK: {
            name = "block.oak";
        } break;

        case BlockType::BLOCK_WATER: {
            name = "block.water";
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

BlockFaceConfig getBlockFaceConfig(BlockType type)
{
    BlockFaceConfig config{};
    switch (type) {
        case BlockType::BLOCK_STONE: {
            GLuint side   = bg::TextureResourceManagerSingleton::get().get("block.stone");
            config.top    = side;
            config.bottom = side;
            config.front  = side;
            config.back   = side;
            config.left   = side;
            config.right  = side;
        } break;

        case BlockType::BLOCK_COBBLE: {
            GLuint cobble = bg::TextureResourceManagerSingleton::get().get("block.cobble");
            config.top    = cobble;
            config.bottom = cobble;
            config.front  = cobble;
            config.back   = cobble;
            config.left   = cobble;
            config.right  = cobble;
        } break;

        case BlockType::BLOCK_LUCKY: {
            GLuint lucky  = bg::TextureResourceManagerSingleton::get().get("block.lucky");
            config.top    = lucky;
            config.bottom = lucky;
            config.front  = lucky;
            config.back   = lucky;
            config.left   = lucky;
            config.right  = lucky;
        } break;

        case BlockType::BLOCK_GRASS: {
            GLuint grass       = bg::TextureResourceManagerSingleton::get().get("block.grass");
            GLuint grassSide   = bg::TextureResourceManagerSingleton::get().get("block.grass.side");
            GLuint grassBottom = bg::TextureResourceManagerSingleton::get().get("block.dirt");
            config.top         = grass;
            config.bottom      = grassBottom;
            config.front       = grassSide;
            config.back        = grassSide;
            config.left        = grassSide;
            config.right       = grassSide;
        } break;

        case BlockType::BLOCK_DIRT: {
            GLuint dirt   = bg::TextureResourceManagerSingleton::get().get("block.dirt");
            config.top    = dirt;
            config.bottom = dirt;
            config.front  = dirt;
            config.back   = dirt;
            config.left   = dirt;
            config.right  = dirt;
        } break;

        case BlockType::BLOCK_OAK: {
            GLuint bottom = bg::TextureResourceManagerSingleton::get().get("block.oak.bottom");
            GLuint side   = bg::TextureResourceManagerSingleton::get().get("block.oak.side");
            config.top    = bottom;
            config.bottom = bottom;
            config.front  = side;
            config.back   = side;
            config.left   = side;
            config.right  = side;
        } break;

        case BlockType::BLOCK_WATER: {
            GLuint side   = bg::TextureResourceManagerSingleton::get().get("block.water");
            config.top    = side;
            config.bottom = side;
            config.front  = side;
            config.back   = side;
            config.left   = side;
            config.right  = side;
        } break;

        case BlockType::BLOCK_AIR:
        default: {
            logError("{} failed. Block type has no face config: {}", __func__, static_cast<int>(type));
            assertr(false);
        } break;
    };

    return config;
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
        case BlockType::BLOCK_OAK:
        case BlockType::BLOCK_STONE:
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
            logWarn("{} failed. Using default sounds instead. Block type has no sound break name: {}", __func__,
                    static_cast<int>(type));
            static auto names =
                std::to_array({ std::string_view{ "block.dig.cobble.1" }, std::string_view{ "block.dig.cobble.2" },

                                std::string_view{ "block.dig.cobble.3" }, std::string_view{ "block.dig.cobble.4" } });
            name = getRandomBlockBreakSoundName(names);
        } break;
    };

    return name;
}

} // namespace bg
