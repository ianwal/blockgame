#include "src/linerenderer.h"

#include "src/app_context.h"
#include "src/block.h"
#include "src/game.h"
#include "src/shaders.h"
#include "src/time.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <glm/ext.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace bg
{

GLuint LineRenderer::vao;
GLuint LineRenderer::vertexbuffer;
GLuint LineRenderer::shaderId;
GLuint LineRenderer::colorId;

void LineRenderer::init()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertexbuffer);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);

    // Reserve space for 2 vertices (start + end), XYZ only
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);

    glBindVertexArray(0);

    shaderId          = bg::ShaderManagerSingleton::get().get("line");
    GLuint blockIndex = glGetUniformBlockIndex(shaderId, "Matrices");
    glUniformBlockBinding(shaderId, blockIndex, 0);

    colorId = glGetUniformLocation(shaderId, "lineColor");
}

void LineRenderer::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

void LineRenderer::render(Line const& line)
{
    glm::vec3 const verts[2]{ line.start, line.end };

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glLineWidth(line.width);

    glUseProgram(shaderId);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    glUseProgram(shaderId);
    glUniform4f(colorId, line.color.r, line.color.g, line.color.b, line.color.a);

    glDrawArrays(GL_LINES, 0, 2);

    glDisable(GL_DEPTH_TEST);
    glLineWidth(1.0f); // reset
    glBindVertexArray(0);
}

} // namespace bg
