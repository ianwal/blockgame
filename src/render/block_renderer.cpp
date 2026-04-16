#include "src/render/block_renderer.h"

#include "src/app_context.h"
#include "src/block.h"
#include "src/game.h"
#include "src/shaders.h"
#include "src/time.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#ifdef BLOCKGAME_ENABLE_TRACY
#include <tracy/Tracy.hpp>
#endif

namespace bg
{

GLuint BlockRenderer::vao;
GLuint BlockRenderer::vertexbuffer;
GLuint BlockRenderer::shaderId;
GLuint BlockRenderer::matrixId;
GLuint BlockRenderer::vbo;

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
    glGenBuffers(1, &vertexbuffer); // cube geometry
    glGenBuffers(1, &vbo);          // instance positions

    glBindVertexArray(vao);

    // cube verts -> vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    constexpr int stride = 5 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(GLfloat)));

    // instance positions -> vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 4096 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*) 0);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);

    shaderId          = bg::ShaderManagerSingleton::get().get("cube");
    GLuint blockIndex = glGetUniformBlockIndex(shaderId, "Matrices");
    glUniformBlockBinding(shaderId, blockIndex, 0);
    glUniform1i(glGetUniformLocation(shaderId, "tex"), 0);
}

void BlockRenderer::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

void BlockRenderer::render(AppContext& ctx, Block const& block, float partialTick)
{
    static_cast<void>(ctx);
    static_cast<void>(block);
    static_cast<void>(partialTick);
    /*
#ifdef BLOCKGAME_ENABLE_TRACY
    ZoneScopedN("SingleBlockRender");
#endif
    if (block.type == BlockType::BLOCK_AIR) {
        return;
    }

    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3((float) block.pos.x, (float) block.pos.y, (float) block.pos.z));

    glBindVertexArray(vao);
    // TODO: Back face culling seems to work but it's not necessary right now so it's disabled.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(shaderId);

    // Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, getBlockTexture(block.type));

    // Note:
    // Setting the texture ID here isn't necessary since the blocks only have one texture. But
    // do it anyway.
    GLuint texId = glGetUniformLocation(shaderId, "tex");
    glUniform1i(texId, 0);

    // Send the transformation to the currently bound shader.
    glUniformMatrix4fv(matrixId, 1, GL_FALSE, glm::value_ptr(model));

    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, 6*6); // 3 vertices per triangle, 2 triangles per face, 6 faces

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(0);
    */
}

void BlockRenderer::render(AppContext& ctx, std::vector<Block> const& blocks, float partialTick)
{
    static_cast<void>(ctx);
    static_cast<void>(partialTick);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);

    std::map<BlockType, std::vector<ivec3>> byType{};
    for (auto const& block : blocks) {
        if (block.type == BlockType::BLOCK_AIR) {
            continue;
        }
        byType[block.type].push_back(tovec3(block.pos));
    }

    glUseProgram(shaderId);
    glBindVertexArray(vao);
    for (auto& [type, positions] : byType) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getBlockTexture(type));
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, positions.size());
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

} // namespace bg
