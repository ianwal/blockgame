#include "src/chunk.h"

#include "src/logger.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <format>
#include <map>
#include <string>
#include <utility>

namespace bg
{

std::string to_string(ChunkPos const& chunkPos)
{
    return std::format("[x: {}, z: {}]", chunkPos.x, chunkPos.z);
}

ChunkPos getChunkPos(glm::vec3 const& worldPos)
{
    ChunkPos chunkPos{};
    chunkPos.x = static_cast<std::int32_t>(worldPos.x) / chunkSize;
    chunkPos.z = static_cast<std::int32_t>(worldPos.z) / chunkSize;
    return chunkPos;
}

glm::vec3 getWorldPos(ChunkPos const& chunkPos, glm::vec3 blockOffset)
{
    return glm::vec3{ (chunkPos.x * chunkSize) + blockOffset.x, blockOffset.y,
                      (chunkPos.z * chunkSize) + blockOffset.z };
}

void ChunkManager::loadChunks(glm::vec3 const& worldPos, std::int32_t chunkLoadRadius, std::uint64_t seed)
{
    ChunkPos startChunkPos = getChunkPos(worldPos);
    for (std::int32_t targetChunkXOffset = -chunkLoadRadius; targetChunkXOffset < chunkLoadRadius;
         ++targetChunkXOffset) {
        for (std::int32_t targetChunkZOffset = -chunkLoadRadius; targetChunkZOffset < chunkLoadRadius;
             ++targetChunkZOffset) {
            ChunkPos const targetChunkPos{ (startChunkPos.x + targetChunkXOffset),
                                           (startChunkPos.z + targetChunkZOffset) };

            if (loadedChunks.contains(targetChunkPos)) {
                continue;
            }

            // Generate chunk
            // logDebug("Generating chunk: {}", to_string(targetChunkPos));

            // If chunk is not loaded, generate it. Chunks are never unloaded currently because unloading checks would
            // require saving them somewhere. They could be unloaded and saved to a vector to avoid iterating over and
            // ignoring them. Minecraft also saves them to disk to save progress.
            // TODO: At some point chunks should be unloaded.
            Chunk chunk{};
            for (int i = 0; i < chunkSize; ++i) {
                for (int j = 0; j < chunkHeight; ++j) {
                    for (int k = 0; k < chunkSize; ++k) {
                        Block block{};
                        int const surface_y = chunkHeight / 2;
                        if (j == surface_y) {
                            block.type = BlockType::BLOCK_GRASS;
                        } else if (j > surface_y) {
                            block.type == BlockType::BLOCK_AIR;
                        } else if (j < surface_y && j >= (surface_y - 5)) {
                            block.type = BlockType::BLOCK_DIRT;
                        } else {
                            block.type = BlockType::BLOCK_COBBLE;
                        }
                        block.pos = getWorldPos(targetChunkPos, glm::vec3{ i, j, k });
                        chunk.blocks.push_back(block);
                    }
                }
            }

            loadedChunks.insert({ targetChunkPos, std::move(chunk) });
        }
    }
}

} // namespace bg
