#pragma once

#include "src/app_context.h"

#include <GL/glew.h>

namespace bg
{

class HandRenderer
{
public:
    // Must be called before render.
    static void init();

    static void cleanup();

    static void render(AppContext& ctx, float partialTick);

private:
    static GLuint vao;
    static GLuint vertexbuffer;
    static GLuint shaderId;
    static GLuint matrixId;
    static GLuint vbo;
};

} // namespace bg
