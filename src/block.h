#pragma once

#include "src/block.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include <cstdint>
#include <string>

namespace bg
{

enum class BlockType : std::uint16_t {
    BLOCK_AIR = 0, ///< Placeholder, not really a block.
    BLOCK_COBBLE,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_LUCKY,

    NUM_BLOCK_TYPES, ///< Meta
};

struct Block {
    glm::vec3 pos{};
    BlockType type{};
};

GLuint getBlockTexture(BlockType type);

std::string getBlockBreakSoundName(BlockType type);

} // namespace bg
