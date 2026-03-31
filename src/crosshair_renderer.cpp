#include "src/crosshair_renderer.h"

#include "src/app_context.h"
#include "src/block.h"
#include "src/game.h"
#include "src/shaders.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

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
    -0.1f,  0.0f,  // horizontal
     0.1f,  0.0f,
     0.0f,  -0.1f, // vertical
     0.0f,   0.1f,
});
// clang-format on

void CrosshairRenderer::init()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    matrixId = glGetUniformLocation(alf::ShaderManagerSingleton::get().get("crosshair"), "projection");
}

void CrosshairRenderer::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

void CrosshairRenderer::render(AppContext& ctx)
{
    auto windowSize = getWindowSize(ctx);
    float cx        = windowSize.width / 2.f;
    float cy        = windowSize.height / 2.f;

    // Crosshair size
    float hw = 12.f;
    float vl = 12.f; // half-widths in pixels

    auto verts = std::to_array({
        cx - hw,
        cy,
        cx + hw,
        cy,
        cx,
        cy - vl,
        cx,
        cy + vl,
    });
    // Orthographic projection matrix to turn normalized device coordinates (NDC) -> Screen Space Coordinates
    glm::mat4 proj =
        glm::ortho(0.f, static_cast<float>(windowSize.width), static_cast<float>(windowSize.height), 0.f, -1.f, 1.f);
    glUseProgram(alf::ShaderManagerSingleton::get().get("crosshair"));
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, glm::value_ptr(proj));

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindVertexArray(vao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
    glLineWidth(2.f);
    glDrawArrays(GL_LINES, 0, 4);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

} // namespace alf
