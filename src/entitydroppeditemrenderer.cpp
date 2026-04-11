#include "src/entitydroppeditemrenderer.h"

#include "src/app_context.h"
#include "src/block.h"
#include "src/entitydroppeditem.h"
#include "src/game.h"
#include "src/item.h"
#include "src/logger.h"
#include "src/shaders.h"
#include "src/time.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <utility>

#ifdef BLOCKGAME_ENABLE_TRACY
#include <tracy/Tracy.hpp>
#endif

// clang-format off
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
// clang-format on

namespace bg
{

// clang-format off
static constexpr GLfloat g_block_vertex_buffer_data[] = {
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

struct InstanceData {
    glm::vec3 pos;
    float scale;
};

EntityDroppedItemRenderer::EntityDroppedItemRenderer()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vertexbuffer); // cube geometry
    glGenBuffers(1, &vbo);          // instance positions

    glBindVertexArray(vao);

    // cube verts -> vertexbuffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_block_vertex_buffer_data), g_block_vertex_buffer_data, GL_STATIC_DRAW);
    constexpr int stride = 5 * sizeof(GLfloat);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(GLfloat)));

    // instance data -> vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 4096 * sizeof(InstanceData), nullptr, GL_DYNAMIC_DRAW);

    // instancePos (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, pos));
    glVertexAttribDivisor(2, 1);

    // instanceScale (location 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*) offsetof(InstanceData, scale));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    shaderId          = bg::ShaderManagerSingleton::get().get("droppedblock");
    GLuint blockIndex = glGetUniformBlockIndex(shaderId, "Matrices");
    glUniformBlockBinding(shaderId, blockIndex, 0);
    glUniform1i(glGetUniformLocation(shaderId, "tex"), 0);
}

EntityDroppedItemRenderer::~EntityDroppedItemRenderer()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

/*
void EntityDroppedItemRenderer::render(AppContext& ctx, Block const& block, float partialTick)
{
static_cast<void>(ctx);
static_cast<void>(block);
static_cast<void>(partialTick);
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
}
*/

void EntityDroppedItemRenderer::render(IEntity const& ientity, float partialTick)
{
    auto const& entity = dynamic_cast<EntityDroppedItem const&>(ientity);

    static_cast<void>(partialTick);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto blockTypeResult = toBlockType(entity.getItemType());
    if (blockTypeResult.has_value()) {
        assertr(blockTypeResult.value() != BlockType::BLOCK_AIR);
        ItemType const itemType = entity.getItemType();
        std::map<BlockType, std::vector<InstanceData>> byType{};
        InstanceData d{};
        d.pos   = entity.getPos(partialTick);
        d.scale = 0.25f;
        byType[blockTypeResult.value()].push_back(d);

        glUseProgram(shaderId);
        glBindVertexArray(vao);

        for (auto& [type, instances] : byType) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, getBlockTexture(type));
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_DYNAMIC_DRAW);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 36, instances.size());
        }
    } else {
        assertr(false);
    }

    glDisable(GL_DEPTH_TEST);
}

} // namespace bg
