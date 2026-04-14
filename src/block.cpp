#include "src/block.h"

#include "src/aabb.h"
#include "src/app_context.h"
#include "src/assertr.h"
#include "src/logger.h"
#include "src/shaders.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include <array>
#include <cassert>
#include <cstdint>
#include <format>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <vector>

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
    return bg::TextureResourceManagerSingleton::get().get(std::string{ App::get().blockManager.getName(type) });
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

        case BlockType::BLOCK_OAKLEAF: {
            GLuint side   = bg::TextureResourceManagerSingleton::get().get("block.oakleaf");
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

static std::string_view getRandomBlockBreakSoundName(std::vector<std::string> soundNames)
{
    static std::mt19937 gen{ std::random_device{}() };
    std::uniform_int_distribution<> dist(0, soundNames.size() - 1);
    return soundNames[dist(gen)];
}

std::string getBlockBreakSoundName(BlockType type)
{
    std::string name{};
    auto const breakSounds = App::get().blockManager.getBreakSounds(type);
    if (breakSounds.empty()) {
        // Block type not added to lookup table. Add it.
        logWarn("Block type has no sound break sounds: {}. Using default sounds instead.", static_cast<int>(type));
        name = "block.dig.dirt.1";
    } else {
        name = getRandomBlockBreakSoundName(breakSounds);
    }

    return name;
}

BlockType BlockManager::getType(std::string const& name)
{
    for (auto const& [blockType, blockName] : names) {
        if (name == blockName) {
            return blockType;
        }
    }
    assertr(false, std::format("No block type found for name: {}", name));
    return {};
}

std::string_view BlockManager::getName(BlockType type)
{
    for (auto const& [blockType, blockName] : names) {
        if (type == blockType) {
            return blockName;
        }
    }

    assertr(false, std::format("No name found for block type: {}", std::to_underlying(type)));
    return {};
}

std::string_view BlockManager::getPrettyName(BlockType type)
{
    for (auto const& [blockType, blockPrettyName] : prettyNames) {
        if (type == blockType) {
            return blockPrettyName;
        }
    }

    assertr(false, std::format("No pretty name found for block type: {}", std::to_underlying(type)));
    return {};
}

std::vector<std::string> BlockManager::getBreakSounds(BlockType type)
{
    for (auto const& [blockType, blockBreakSounds] : breakSounds) {
        if (type == blockType) {
            return blockBreakSounds;
        }
    }

    logWarn("No break sounds found for block type: {}", std::to_underlying(type));
    return {};
}

BlockManager& BlockManager::load(std::string const& name, std::uint16_t id, std::string const& prettyName,
                                 std::vector<std::string> const& blockBreakSounds)
{
    auto const type = static_cast<BlockType>(id);
    assertr(!names.contains(type), name);
    assertr(!prettyNames.contains(type), name);
    assertr(!breakSounds.contains(type), name);

    names[type]       = name;
    prettyNames[type] = prettyName;
    breakSounds[type] = blockBreakSounds;
    return *this;
}

} // namespace bg
