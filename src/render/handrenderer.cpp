#include "src/render/handrenderer.h"

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

GLuint HandRenderer::vao;
GLuint HandRenderer::vertexbuffer;
GLuint HandRenderer::shaderId;
GLuint HandRenderer::matrixId;
GLuint HandRenderer::vbo;

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

void HandRenderer::init()
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

void HandRenderer::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

void HandRenderer::render(AppContext& ctx, float partialTick)
{
    GLint saveProgram{};
    glGetIntegerv(GL_CURRENT_PROGRAM, &saveProgram);

    // Use hand shader instead of cube shader
    GLuint handShaderId = bg::ShaderManagerSingleton::get().get("hand");
    glUseProgram(handShaderId);

    auto const heldItemResult = ctx.game->playerInventory.getCurrentHeldItem();
    auto const renderAsHand   = !heldItemResult.has_value();
    auto const renderAsBlock  = !renderAsHand && toBlockType(heldItemResult->item).has_value();

    auto& camera = ctx.game->player.fpCamera;

    auto windowSize                 = getWindowSize(ctx);
    float const fov                 = 90.0f;
    float const near_clipping_plane = 0.1f;   // Keep as big as possible or you'll get precision issues.
    float const far_clipping_plane  = 100.0f; // Keep as small as possible.
    glm::vec3 cameraPos = ctx.game->player.currentCamera->getPos(partialTick); // Camera position in world space.
    glm::vec3 cameraFront =
        ctx.game->player.currentCamera->getFront(partialTick); // Front of the camera in world space.
    glm::vec3 cameraUp = ctx.game->player.currentCamera->getUp();

    glm::mat4 projection = glm::perspective(
        glm::radians(fov), static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height),
        near_clipping_plane, far_clipping_plane);
    glm::mat4 view = glm::lookAt(cameraPos,               // Camera is at this coordinate in world space
                                 cameraPos + cameraFront, // Camera looks at this coordinate.
                                 cameraUp                 // Camera head is up (0, -1, 0) would be looking upside-down)
    );

    glm::vec3 forward = glm::normalize(camera.getFront(partialTick));
    glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up      = glm::normalize(glm::cross(right, forward));

    glm::vec3 fixedRight = right;
    glm::vec3 fixedUp    = up;

    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), right);
    rot           = glm::rotate(rot, glm::radians(-10.0f), up);
    rot           = glm::rotate(rot, glm::radians(10.0f), forward);

    float const swingProgress = ctx.game->player.getSwingProgress(partialTick);
    float const f1            = glm::sin(swingProgress * glm::pi<float>());
    float const f2            = glm::sin(swingProgress * swingProgress * glm::pi<float>());

    rot = glm::rotate(rot, glm::radians(-60.0f) * f2, right); // punch forward (pitch)

    glm::vec3 const vel = ctx.game->player.getVelocity(partialTick);

    forward = glm::normalize(glm::vec3(rot * glm::vec4(forward, 0.0f)));
    right   = glm::normalize(glm::vec3(rot * glm::vec4(right, 0.0f)));
    up      = glm::normalize(glm::vec3(rot * glm::vec4(up, 0.0f)));

    // hand position in world space
    glm::vec3 o = camera.getPos(partialTick) + (vel * 0.1f) + (fixedUp * -0.1f) + (fixedRight * 0.4f) +
                  (forward * 0.05f) + ((right * 0.1f) * (2.0f * -f1)) + up * (-0.6f + f1 * 0.05f - f2 * 0.15f);

    // size
    float width{};
    float height{};
    float depth{};
    if (renderAsHand) {
        width  = 0.3f;
        height = 0.2f;
        depth  = 0.5f;
    } else {
        // Block
        width  = 0.3f;
        height = 0.3f;
        depth  = 0.3f;
    }

    // basis vectors scaled to cube size
    glm::vec3 r = right * width;
    glm::vec3 u = up * height;
    glm::vec3 f = forward * depth;

    glm::mat4 model{};
    if (renderAsBlock)
        // If rendering as block, rotate by 90 degrees so that the top of the block the right way correctly.
        // TODO: This might not actually be correct, it might need to shift all the faces here to the right.
        model = glm::mat4(glm::vec4(r, 0.0f), glm::vec4(f, 0.0f), glm::vec4(u, 0.0f), glm::vec4(o, 1.0f));
    else {
        model = glm::mat4(glm::vec4(r, 0.0f), glm::vec4(u, 0.0f), glm::vec4(f, 0.0f), glm::vec4(o, 1.0f));
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // TODO:
    // Non-block types should be shown in hand, but this function only renders blocks and hand right now so no need to
    // handle this here.
    GLuint textureId{};
    BlockFaceConfig faceConfig{};
    if (renderAsBlock) {
        auto const blockType = toBlockType(heldItemResult->item).value();
        textureId            = getBlockTexture(blockType);
        faceConfig           = getBlockFaceConfig(blockType);
    } else if (renderAsHand) {
        textureId = TextureResourceManagerSingleton::get().get("player.hand");
    } else {
        // TODO
    }

    // Face textures and their vertex ranges
    // Front face (0), Back face (1), Right face (2), Left face (3), Top face (4), Bottom face (5)
    const GLuint faceTextures[]   = { faceConfig.front, faceConfig.back, faceConfig.right,
                                      faceConfig.left,  faceConfig.top,  faceConfig.bottom };
    const int faceStartVertices[] = { 0, 6, 12, 18, 24, 30 };
    const int verticesPerFace     = 6;

    // Clear the buffer bit so that the hand is always rendered in front of the world.
    glClear(GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);

    // Set up uniforms for the hand shader
    GLuint modelId      = glGetUniformLocation(handShaderId, "model");
    GLuint projectionId = glGetUniformLocation(handShaderId, "projection");
    GLuint viewId       = glGetUniformLocation(handShaderId, "view");

    glUniformMatrix4fv(modelId, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(projectionId, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewId, 1, GL_FALSE, glm::value_ptr(view));
    glUniform1i(glGetUniformLocation(handShaderId, "tex"), 0);

    for (int face = 0; face < 6; ++face) {
        if (renderAsBlock) {
            glBindTexture(GL_TEXTURE_2D, faceTextures[face]);
        } else {
            glBindTexture(GL_TEXTURE_2D, textureId);
        }
        glDrawArrays(GL_TRIANGLES, faceStartVertices[face], verticesPerFace);
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(saveProgram);
}

} // namespace bg
