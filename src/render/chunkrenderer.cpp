#include "src/render/chunkrenderer.h"

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

GLuint ChunkRenderer::vao;
GLuint ChunkRenderer::vertexbuffer;
GLuint ChunkRenderer::shaderId;
GLuint ChunkRenderer::matrixId;
GLuint ChunkRenderer::vbo;
std::vector<ChunkPos> ChunkRenderer::dirtyChunks;
std::unordered_map<ChunkPos, std::map<BlockType, std::map<BlockFaceConfig, std::vector<glm::vec3>>>>
    ChunkRenderer::blockPosData;

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

void ChunkRenderer::init()
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

void ChunkRenderer::cleanup()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vertexbuffer);
}

void ChunkRenderer::render(AppContext& ctx, ChunkManager const& chunkManager, float partialTick)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);

    for (ChunkPos const& chunkPos : dirtyChunks) {
        // Create the data or clear the data if it already exists.
        blockPosData[chunkPos] = {};

        assertr(chunkManager.loadedChunks.contains(chunkPos));
        auto const& chunk = *chunkManager.loadedChunks.find(chunkPos)->second;
        for (std::size_t i = 0; i < chunk.blocks.size(); ++i) {
            Block block{};
            block.type = chunk.blocks[i];
            if (block.type == BlockType::BLOCK_AIR) {
                continue;
            }
            block.pos                  = ChunkManager::getBlockWorldPos(chunkPos, i);
            BlockFaceConfig faceConfig = getBlockFaceConfig(block.type);
            blockPosData[chunkPos][block.type][faceConfig].push_back(glm::vec3(block.pos));
        }
    }

    dirtyChunks.clear(); // Chunk data rebuilt, so clear it.

    // Get any loaded chunks
    for (auto const& [chunkPos, chunk] : chunkManager.loadedChunks) {
        if (!blockPosData.contains(chunkPos)) {
            for (std::size_t i = 0; i < chunk->blocks.size(); ++i) {
                Block block{};
                block.type = chunk->blocks[i];
                if (block.type == BlockType::BLOCK_AIR) {
                    continue;
                }
                block.pos                  = ChunkManager::getBlockWorldPos(chunkPos, i);
                BlockFaceConfig faceConfig = getBlockFaceConfig(block.type);
                blockPosData[chunkPos][block.type][faceConfig].push_back(glm::vec3(block.pos));
            }
        }
    }

    // Remove any unloaded chunks.
    // TODO: This is clearly inefficient and requires multiple unnecessary lookups.
    std::vector<ChunkPos> unloadedChunks{};
    for (auto const& [chunkPos, chunk] : blockPosData) {
        if (!chunkManager.loadedChunks.contains(chunkPos)) {
            unloadedChunks.push_back(chunkPos);
        }
    }
    for (auto const& chunkPos : unloadedChunks) {
        blockPosData.erase(blockPosData.find(chunkPos));
    }

    // Note: I don't know why, but rendering a chunk at a time is ~30fps faster than rendering all chunks at once.
    ChunkPos const playerChunkPos = getChunkPos(ctx.game->player.getPos(partialTick));
    glm::vec2 const playerChunkPosVec{ playerChunkPos.x, playerChunkPos.z };
    glUseProgram(shaderId);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);

    for (auto const& [chunkPos, data] : blockPosData) {
        for (auto& [type, faceConfigData] : data) {
            for (auto& [faceConfig, positions] : faceConfigData) {
                if (positions.empty()) {
                    continue;
                }
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);

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
            }
        }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void ChunkRenderer::markChunkDirty(ChunkPos pos)
{
    dirtyChunks.push_back(pos);
}

} // namespace bg
