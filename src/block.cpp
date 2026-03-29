#include "src/block.h"

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

        default: {
            // Block type texture not added to lookup table. Add it.
            assert(false);
        } break;
    };

    return alf::TextureResourceManagerSingleton::get().get(name);
}

} // namespace alf
