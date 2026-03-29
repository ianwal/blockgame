#pragma once

#include <GL/glew.h>
#include <glm/vec3.hpp>

#include <cstdint>

namespace Alf
{

enum class BlockType : std::uint32_t {
    BLOCK_COBBLE = 0,
    BLOCK_LUCKY,
};

struct Block {
    BlockType type{};
    glm::vec3 pos{};
};

GLuint getBlockTexture(BlockType type);

} // namespace Alf
