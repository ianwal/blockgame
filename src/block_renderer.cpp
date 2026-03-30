#include "src/block_renderer.h"
#include "src/app_context.h"
#include "src/block.h"
#include "src/game.h"
#include "src/shaders.h"
#include "src/time.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdlib>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace alf
{

GLuint BlockRenderer::vao;
GLuint BlockRenderer::vertexbuffer;
GLuint BlockRenderer::matrixId;

// clang-format off
static constexpr GLfloat g_vertex_buffer_data[] = {
    // X      Y      Z      U     V
    // Front (z = +0.5)
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    // Back (z = -0.5)
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    // Right (x = +0.5)
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    // Left (x = -0.5)
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    // Top (y = +0.5)
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    // Bottom (y = -0.5)
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,
};
// clang-format on

void BlockRenderer::init()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // This will identify our vertex buffer
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // stride = 5 floats (3 pos + 2 uv)
    constexpr int stride = 5 * sizeof(GLfloat);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);

    // UV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(GLfloat)));
    glBindVertexArray(0);

    // Get a handle for the MVP uniform
    matrixId = glGetUniformLocation(alf::ShaderManagerSingleton::get().get("cube"), "MVP");
}

void BlockRenderer::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

void BlockRenderer::render(AppContext& ctx, Block const& block)
{
    glBindVertexArray(vao);
    // TODO: Back face culling seems to work but it's not necessary right now so it's disabled.
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(alf::ShaderManagerSingleton::get().get("cube"));

    int width{};
    int height{};
    auto windowSize = getWindowSize(ctx);
    width           = windowSize.width;
    height          = windowSize.height;

    float const fov                 = 90.0f;
    float const near_clipping_plane = 0.1f;                  // Keep as big as possible or you'll get precision issues.
    float const far_clipping_plane  = 100.0f;                // Keep as small as possible.
    glm::vec3 cameraPos             = ctx.game->cameraPos;   // Camera position in world space.
    glm::vec3 cameraFront           = ctx.game->cameraFront; // Front of the camera in world space.

    glm::mat4 projection_mat =
        glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), near_clipping_plane,
                         far_clipping_plane);
    glm::mat4 view_mat = glm::lookAt(cameraPos,               // Camera is at this coordinate in world space
                                     cameraPos + cameraFront, // Camera looks at this coordinate.
                                     glm::vec3(0, 1, 0) // Camera head is up (0, -1, 0) would be looking upside-down)
    );

    glm::mat4 model_mat = glm::mat4(1.0f); // identity matrix so model will be at the origin
    model_mat = glm::translate(model_mat, glm::vec3((float) block.pos.x, (float) block.pos.y, (float) block.pos.z));
    glm::mat4 model_view_projection = projection_mat * view_mat * model_mat;

    // Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, getBlockTexture(block.type));

    // Note:
    // Setting the texture here isn't necessary since the blocks only have one texture. But
    // do it anyway.
    GLuint texId = glGetUniformLocation(alf::ShaderManagerSingleton::get().get("cube"), "tex");
    glUniform1i(texId, 0);

    // Send the transformation to the currently bound shader in the MVP uniform
    // Each model will have a different MVP (well... a different M) so this will be done in the main rendering loop.
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, &model_view_projection[0][0]);

    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, 12 * 6); // 12*6 indices starting at 0 -> 12 triangles -> 6 squares

    glDisable(GL_DEPTH_TEST);
    // glDisable(GL_CULL_FACE);
    glBindVertexArray(0);
}

} // namespace alf
