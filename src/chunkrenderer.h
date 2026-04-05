#pragma once

#include "src/app_context.h"
#include "src/block.h"
#include "src/chunk.h"

#include <GL/glew.h>

namespace bg
{

class ChunkRenderer
{
public:
    // Must be called before render.
    static void init();

    static void cleanup();

    static void render(AppContext& ctx, ChunkManager const& chunkManager, float partialTick);

private:
    static GLuint vao;
    static GLuint vertexbuffer;
    static GLuint shaderId;
    static GLuint matrixId;
    static GLuint vbo;
};

} // namespace bg
