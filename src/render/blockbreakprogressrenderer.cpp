#include "src/render/blockbreakprogressrenderer.h"

#include "src/app_context.h"
#include "src/block.h"
#include "src/game.h"
#include "src/render/chunkrenderer.h"
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

#include <format>

#ifdef BLOCKGAME_ENABLE_TRACY
#include <tracy/Tracy.hpp>
#endif

namespace bg
{

GLuint BlockBreakProgressRenderer::vao;
GLuint BlockBreakProgressRenderer::vertexbuffer;
GLuint BlockBreakProgressRenderer::shaderId;
GLuint BlockBreakProgressRenderer::matrixId;
GLuint BlockBreakProgressRenderer::vbo;

// clang-format off
static constexpr GLfloat g_vertex_buffer_data[] = {
    // X      Y      Z      U     V     FaceID
    // Front (z = +0.5) - Face 0
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f,
    // Back (z = -0.5) - Face 1
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,
    0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,
    0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  1.0f,
    // Right (x = +0.5) - Face 2
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  2.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  2.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  2.0f,
    0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  2.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  2.0f,
    0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  2.0f,
    // Left (x = -0.5) - Face 3
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  3.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  3.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  3.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  3.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  3.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  3.0f,
    // Top (y = +0.5) - Face 4
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  4.0f,
    0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  4.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  4.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  4.0f,
    0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  4.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  4.0f,
    // Bottom (y = -0.5) - Face 5
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  5.0f,
    0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  5.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  5.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  5.0f,
    0.5f, -0.5f,  0.5f,  1.0f, 1.0f,  5.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  5.0f,
};
// clang-format on

void BlockBreakProgressRenderer::init()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertexbuffer); // cube geometry
    glGenBuffers(1, &vbo);          // instance positions

    glBindVertexArray(vao);

    // cube verts -> vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    constexpr int stride = 6 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*) (5 * sizeof(GLfloat)));

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

void BlockBreakProgressRenderer::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

GLuint getBlockBreakTexture(float breakProgress)
{
    if (breakProgress > 0.0f && breakProgress < 0.1f) {
        return bg::TextureResourceManagerSingleton::get().get("break.0");
    } else if (breakProgress >= 0.1f && breakProgress < 0.2f) {
        return bg::TextureResourceManagerSingleton::get().get("break.1");
    } else if (breakProgress >= 0.2f && breakProgress < 0.3f) {
        return bg::TextureResourceManagerSingleton::get().get("break.2");
    } else if (breakProgress >= 0.3f && breakProgress < 0.4f) {
        return bg::TextureResourceManagerSingleton::get().get("break.3");
    } else if (breakProgress >= 0.4f && breakProgress < 0.5f) {
        return bg::TextureResourceManagerSingleton::get().get("break.4");
    } else if (breakProgress >= 0.5f && breakProgress < 0.6f) {
        return bg::TextureResourceManagerSingleton::get().get("break.5");
    } else if (breakProgress >= 0.6f && breakProgress < 0.7f) {
        return bg::TextureResourceManagerSingleton::get().get("break.6");
    } else if (breakProgress >= 0.7f && breakProgress < 0.8f) {
        return bg::TextureResourceManagerSingleton::get().get("break.7");
    } else if (breakProgress >= 0.8f && breakProgress < 0.9f) {
        return bg::TextureResourceManagerSingleton::get().get("break.8");
    } else if (breakProgress >= 0.9f) {
        return bg::TextureResourceManagerSingleton::get().get("break.9");
    }

    assertr(false, std::format("{}", breakProgress));
    return 0;
}

void BlockBreakProgressRenderer::render()
{
    auto const& damageTracker = App::get().game->blockDamageTracker;
    if (!(damageTracker.getDamageProgress() > 0.0f)) {
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // Offset the textures a bit towards the camera to avoid z-buffer fighting with the block texture.
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f); // push overlay slightly toward camera

    glUseProgram(shaderId);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);

    std::vector<glm::vec3> positions{};
    positions.push_back(damageTracker.getCurrentBlock().pos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);

    GLuint const breakBlockTexture = getBlockBreakTexture(damageTracker.getDamageProgress());
    BlockFaceConfig faceConfig{ breakBlockTexture, breakBlockTexture, breakBlockTexture,
                                breakBlockTexture, breakBlockTexture, breakBlockTexture };

    // Face textures and their vertex ranges
    // Front face (0), Back face (1), Right face (2), Left face (3), Top face (4), Bottom face (5)
    const GLuint faceTextures[]   = { faceConfig.front, faceConfig.back, faceConfig.right,
                                      faceConfig.left,  faceConfig.top,  faceConfig.bottom };
    const int faceStartVertices[] = { 0, 6, 12, 18, 24, 30 };
    const int verticesPerFace     = 6;

    for (int face = 0; face < 6; ++face) {
        glBindTexture(GL_TEXTURE_2D, faceTextures[face]);
        glDrawArraysInstanced(GL_TRIANGLES, faceStartVertices[face], verticesPerFace, positions.size());
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

} // namespace bg
