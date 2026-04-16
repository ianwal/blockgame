#pragma once

#include "src/app_context.h"
#include "src/block.h"
#include "src/chunk.h"

#include <GL/glew.h>

#include <map>
#include <vector>

namespace bg
{

class ChunkRenderer
{
public:
    // Must be called before render.
    static void init();

    static void cleanup();

    static void render(AppContext& ctx, ChunkManager const& chunkManager, float partialTick);

    static void markChunkDirty(ChunkPos pos);

private:
    static GLuint vao;
    static GLuint vertexbuffer;
    static GLuint shaderId;
    static GLuint matrixId;
    static GLuint vbo;

    static std::vector<ChunkPos> dirtyChunks;

    /// Map of chunks to render and the blocks inside of them.
    ///
    /// The second map is keyed by block type, and the third by face textures.
    /// Blocks are rendered in batches by their block type and face texture configuration for each chunk.
    static std::unordered_map<ChunkPos, std::map<BlockType, std::map<BlockFaceConfig, std::vector<glm::vec3>>>>
        blockPosData;
};

} // namespace bg
