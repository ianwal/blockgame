#include "src/crosshair_renderer.h"

#include "src/app_context.h"
#include "src/block.h"
#include "src/game.h"
#include "src/shaders.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <array>
#include <cassert>
#include <climits>
#include <cstdlib>

namespace alf
{

GLuint CrosshairRenderer::vao;
GLuint CrosshairRenderer::vertexbuffer;
GLuint CrosshairRenderer::matrixId;

// clang-format off
static constexpr auto crosshair_vertex_buffer_data = std::to_array({
    -0.013f,  0.0f,  // horizontal
     0.013f,  0.0f,
     0.0f,  -0.0225f, // vertical
     0.0f,   0.0225f,
});
// clang-format on

void CrosshairRenderer::init()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, crosshair_vertex_buffer_data.size() * CHAR_BIT, crosshair_vertex_buffer_data.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
}

void CrosshairRenderer::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

void CrosshairRenderer::render(AppContext& ctx)
{
    glBindVertexArray(vao);
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    glEnable(GL_BLEND);
    glUseProgram(alf::ShaderManagerSingleton::get().get("crosshair"));
    glLineWidth(2.f);
    glDrawArrays(GL_LINES, 0, 4);
    glDisable(GL_BLEND);
}

} // namespace alf
