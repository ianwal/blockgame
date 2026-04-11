#pragma once

#include "src/aabb.h"
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

/// Integer vec3.
using ivec3 = glm::tvec3<std::int32_t>;

/// Convert integer vector to float vector.
constexpr glm::vec3 tovec3(ivec3 const& v)
{
    return glm::vec3{ v.x, v.y, v.z };
}

/// Get the block position for the world position.
///
/// This is an information lossy operation. Many world points inside a block can be mapped to the single block position.
constexpr ivec3 toBlockPos(glm::vec3 const& worldPos)
{
    return glm::round(worldPos);
}

struct Block {
    ivec3 pos{};
    BlockType type{};
};

AABB getBlockAABB(Block const& block);

GLuint getBlockTexture(BlockType type);

std::string getBlockBreakSoundName(BlockType type);

} // namespace bg
