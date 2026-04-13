#pragma once

#include "src/aabb.h"

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include <cstdint>
#include <functional>
#include <string>

namespace bg
{

enum class BlockType : std::uint16_t {
    BLOCK_AIR = 0, ///< Placeholder, not really a block.
    BLOCK_STONE,
    BLOCK_COBBLE,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_LUCKY,
    BLOCK_OAK,
    BLOCK_WATER,

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

struct BlockFaceConfig {
    GLuint top;
    GLuint bottom;
    GLuint front;
    GLuint back;
    GLuint left;
    GLuint right;

    bool operator==(const BlockFaceConfig& other) const = default;

    bool operator<(const BlockFaceConfig& other) const
    {
        if (top != other.top)
            return top < other.top;
        if (bottom != other.bottom)
            return bottom < other.bottom;
        if (front != other.front)
            return front < other.front;
        if (back != other.back)
            return back < other.back;
        if (left != other.left)
            return left < other.left;
        return right < other.right;
    }
};

} // namespace bg

namespace std
{
template <>
struct hash<bg::BlockFaceConfig> {
    size_t operator()(const bg::BlockFaceConfig& cfg) const
    {
        return std::hash<GLuint>()(cfg.top) ^ (std::hash<GLuint>()(cfg.bottom) << 1) ^
               (std::hash<GLuint>()(cfg.front) << 2) ^ (std::hash<GLuint>()(cfg.back) << 3) ^
               (std::hash<GLuint>()(cfg.left) << 4) ^ (std::hash<GLuint>()(cfg.right) << 5);
    }
};
} // namespace std

namespace bg
{

AABB getBlockAABB(Block const& block);

GLuint getBlockTexture(BlockType type);

BlockFaceConfig getBlockFaceConfig(BlockType type);

std::string getBlockBreakSoundName(BlockType type);

} // namespace bg
