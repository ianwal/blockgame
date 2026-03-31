#pragma once

#include "src/app_context.h"
#include "src/block.h"

#include <GL/glew.h>

namespace alf
{

class BlockRenderer
{
public:
    // Must be called before render.
    static void init();

    static void cleanup();

    static void render(AppContext& ctx, Block const& block);

private:
    static GLuint vao;
    static GLuint vertexbuffer;
    static GLuint matrixId;
};

} // namespace alf
