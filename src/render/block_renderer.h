#pragma once

#include "src/app_context.h"
#include "src/block.h"

#include <GL/glew.h>

namespace bg
{

class BlockRenderer
{
public:
    // Must be called before render.
    static void init();

    static void cleanup();

    static void render(AppContext& ctx, Block const& block, float partialTick);

    static void render(AppContext& ctx, std::vector<Block> const& blocks, float partialTick);

private:
    static GLuint vao;
    static GLuint vertexbuffer;
    static GLuint shaderId;
    static GLuint matrixId;
    static GLuint vbo;
};

} // namespace bg
