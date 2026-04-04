#include "src/block.h"

#include "src/logger.h"
#include "src/shaders.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include <cassert>
#include <cstdint>
#include <string>

namespace alf
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

        case BlockType::BLOCK_AIR: {
            assert(false);
            name = "block.air";
        } break;

        default: {
            // Block type texture not added to lookup table. Add it.
            logError("{} failed. Block type has no texture: {}", __func__, static_cast<int>(type));
            assert(false);
        } break;
    };

    return alf::TextureResourceManagerSingleton::get().get(name);
}

} // namespace alf
