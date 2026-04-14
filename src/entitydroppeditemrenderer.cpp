#include "src/entitydroppeditemrenderer.h"

#include "src/app_context.h"
#include "src/block.h"
#include "src/entitydroppeditem.h"
#include "src/game.h"
#include "src/item.h"
#include "src/logger.h"
#include "src/shaders.h"
#include "src/time.h"
#include "src/utils.h"

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
    glm::mat3 rot;
    float _pad;
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

    // instanceRotation (location 4)
    for (int i = 0; i < 3; i++) {
        glVertexAttribPointer(4 + i, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData),
                              (void*) (offsetof(InstanceData, rot) + i * sizeof(glm::vec3)));
        glVertexAttribDivisor(4 + i, 1);
        glEnableVertexAttribArray(4 + i);
    }

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

void EntityDroppedItemRenderer::render(IEntity const& ientity, float partialTick)
{
    auto const& entity = dynamic_cast<EntityDroppedItem const&>(ientity);

    static_cast<void>(partialTick);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    auto const blockTypeResult = toBlockType(entity.getItemType());
    auto const renderAsBlock   = blockTypeResult.has_value();
    if (renderAsBlock) {
        BlockType const blockType = blockTypeResult.value();
        assertr(blockType != BlockType::BLOCK_AIR);
        std::map<BlockType, std::vector<InstanceData>> byType{};
        InstanceData d{};
        d.pos   = entity.getRenderPos(partialTick);
        d.rot   = glm::mat3_cast(glm::quat(entity.getRenderRotation(partialTick)));
        d.scale = 0.25f;
        byType[blockType].push_back(d);

        glUseProgram(shaderId);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        for (auto& [type, instances] : byType) {
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(InstanceData), instances.data(), GL_DYNAMIC_DRAW);

            BlockFaceConfig faceConfig{};
            faceConfig = getBlockFaceConfig(type);
            // Face textures and their vertex ranges
            // Front face (0), Back face (1), Right face (2), Left face (3), Top face (4), Bottom face (5)
            const GLuint faceTextures[]   = { faceConfig.front, faceConfig.back, faceConfig.right,
                                              faceConfig.left,  faceConfig.top,  faceConfig.bottom };
            const int faceStartVertices[] = { 0, 6, 12, 18, 24, 30 };
            const int verticesPerFace     = 6;

            for (int face = 0; face < 6; ++face) {
                glBindTexture(GL_TEXTURE_2D, faceTextures[face]);
                glDrawArrays(GL_TRIANGLES, faceStartVertices[face], verticesPerFace);
            }
        }
    } else {
        assertr(false);
    }

    glDisable(GL_DEPTH_TEST);
}

} // namespace bg
