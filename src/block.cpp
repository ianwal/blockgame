#include "src/block.h"

#include "src/app_context.h"
#include "src/logger.h"
#include "src/shaders.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include <cassert>
#include <cstdint>
#include <string>

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

std::string getBlockBreakSoundName(BlockType type)
{
    std::string name{};
    switch (type) {
        case BlockType::BLOCK_COBBLE: {
            name = "block.dig.cobble.1";
        } break;

        case BlockType::BLOCK_LUCKY: {
            name = "block.dig.cobble.1"; // TODO
        } break;

        case BlockType::BLOCK_GRASS: {
            name = "block.dig.grass.1";
        } break;

        case BlockType::BLOCK_DIRT: {
            name = "block.dig.dirt.1";
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
