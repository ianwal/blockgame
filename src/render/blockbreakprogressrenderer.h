#pragma once

#include <GL/glew.h>

namespace bg
{

class BlockBreakProgressRenderer
{
public:
    // Must be called before render.
    static void init();

    static void cleanup();

    static void render();

private:
    static GLuint vao;
    static GLuint vertexbuffer;
    static GLuint shaderId;
    static GLuint matrixId;
    static GLuint vbo;
};

} // namespace bg
