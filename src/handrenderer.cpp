#include "src/handrenderer.h"

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
    glUseProgram(0);

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

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projection));

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(view));

    glm::vec3 forward = glm::normalize(camera.getFront(partialTick));
    glm::vec3 right   = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up      = glm::normalize(glm::cross(right, forward));

    // hand position in world space
    glm::vec3 o = camera.getPos(partialTick) + forward * 0.2f + right * 0.2f + up * -0.4f;

    // cube size
    float s = 0.2f;

    // basis vectors scaled to cube size
    glm::vec3 r = right * s;
    glm::vec3 u = up * s;
    glm::vec3 f = forward * s;

    // 8 corners of the oriented cube
    glm::vec3 ldb = o;
    glm::vec3 rdb = o + r;
    glm::vec3 lub = o + u;
    glm::vec3 rub = o + r + u;
    glm::vec3 ldf = o + f;
    glm::vec3 rdf = o + r + f;
    glm::vec3 luf = o + u + f;
    glm::vec3 ruf = o + r + u + f;

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.68, 0.42, 0.23);

    glBegin(GL_QUADS);
    // front
    glVertex3fv(glm::value_ptr(ldf));
    glVertex3fv(glm::value_ptr(rdf));
    glVertex3fv(glm::value_ptr(ruf));
    glVertex3fv(glm::value_ptr(luf));
    // back
    glVertex3fv(glm::value_ptr(rdb));
    glVertex3fv(glm::value_ptr(ldb));
    glVertex3fv(glm::value_ptr(lub));
    glVertex3fv(glm::value_ptr(rub));
    // left
    glVertex3fv(glm::value_ptr(ldb));
    glVertex3fv(glm::value_ptr(ldf));
    glVertex3fv(glm::value_ptr(luf));
    glVertex3fv(glm::value_ptr(lub));
    // right
    glVertex3fv(glm::value_ptr(rdf));
    glVertex3fv(glm::value_ptr(rdb));
    glVertex3fv(glm::value_ptr(rub));
    glVertex3fv(glm::value_ptr(ruf));
    // top
    glVertex3fv(glm::value_ptr(luf));
    glVertex3fv(glm::value_ptr(ruf));
    glVertex3fv(glm::value_ptr(rub));
    glVertex3fv(glm::value_ptr(lub));
    // bottom
    glVertex3fv(glm::value_ptr(ldb));
    glVertex3fv(glm::value_ptr(rdb));
    glVertex3fv(glm::value_ptr(rdf));
    glVertex3fv(glm::value_ptr(ldf));
    glEnd();

    glUseProgram(saveProgram);
}

} // namespace bg
