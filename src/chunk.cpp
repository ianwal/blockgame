#include "src/chunk.h"

#include "src/block.h"
#include "src/logger.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <format>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace bg
{

std::string to_string(ChunkPos const& chunkPos)
{
    return std::format("[x: {}, z: {}]", chunkPos.x, chunkPos.z);
}

ChunkPos getChunkPos(glm::vec3 const& worldPos)
{
    ChunkPos chunkPos{};
    chunkPos.x = static_cast<std::int32_t>(std::floor(worldPos.x / chunkSize));
    chunkPos.z = static_cast<std::int32_t>(std::floor(worldPos.z / chunkSize));
    return chunkPos;
}

glm::vec3 getWorldPos(ChunkPos const& chunkPos, glm::vec3 blockOffset)
{
    return glm::vec3{ (chunkPos.x * chunkSize) + blockOffset.x, blockOffset.y,
                      (chunkPos.z * chunkSize) + blockOffset.z };
}

void ChunkManager::updateChunks(glm::vec3 const& worldPos, std::int32_t const chunkLoadRadius, std::uint64_t const seed)
{
    static_cast<void>(seed); // TODO: Use seed for chunk gen.

    loadedChunks.clear();
    ChunkPos startChunkPos = getChunkPos(worldPos);
    for (std::int32_t targetChunkXOffset = -chunkLoadRadius; targetChunkXOffset < chunkLoadRadius;
         ++targetChunkXOffset) {
        for (std::int32_t targetChunkZOffset = -chunkLoadRadius; targetChunkZOffset < chunkLoadRadius;
             ++targetChunkZOffset) {
            ChunkPos const targetChunkPos{ (startChunkPos.x + targetChunkXOffset),
                                           (startChunkPos.z + targetChunkZOffset) };
            if (allChunks.contains(targetChunkPos)) {
                loadedChunks.insert({ targetChunkPos, allChunks[targetChunkPos].get() });
            } else {
                // Generate chunk
                // logDebug("Generating chunk: {}", to_string(targetChunkPos));

                float const freq = 0.2;
                // If chunk is not loaded, generate it.
                auto chunk = std::make_unique<Chunk>();
                for (int i = 0; i < chunkSize; ++i) {
                    for (int j = 0; j < chunkHeight; ++j) {
                        for (int k = 0; k < chunkSize; ++k) {
                            Block block{};
                            block.pos        = getWorldPos(targetChunkPos, glm::vec3{ i, j, k });
                            float sumOffsetX = 0;
                            float sumOffsetZ = 0;
                            for (int octave = 1; octave < 3; ++octave) {
                                float octaveScaled = octave * 4;
                                sumOffsetX += (std::sin(((octaveScaled) *block.pos.x * freq) / (octaveScaled * 4)));
                                sumOffsetZ += (std::sin(((octaveScaled) *block.pos.z * freq) / (octaveScaled * 4)));
                            }
                            float surfaceY = 50 + ((sumOffsetX + sumOffsetZ) * 3);

                            if (block.pos.y < surfaceY) {
                                if ((block.pos.y < surfaceY - 1.0) && (block.pos.y > surfaceY - 2.0)) {
                                    block.type = BlockType::BLOCK_DIRT;
                                } else if (block.pos.y < (surfaceY - 5.0)) {
                                    block.type = BlockType::BLOCK_COBBLE;
                                } else {
                                    block.type = BlockType::BLOCK_GRASS;
                                }
                            } else {
                                block.type = BlockType::BLOCK_AIR;
                            }
                            chunk->blocks.push_back(block);
                        }
                    }
                }
                allChunks.insert({ targetChunkPos, std::move(chunk) });
            }
        }
    }
}

std::vector<Block const*> ChunkManager::getNearbyBlocks(glm::vec3 const& worldPos, float const radius) const
{
    std::vector<Block const*> result{};

    glm::vec3 const minPos = worldPos - radius;
    glm::vec3 const maxPos = worldPos + radius;

    ChunkPos const minChunk = getChunkPos(minPos);
    ChunkPos const maxChunk = getChunkPos(maxPos);

    for (std::int32_t cx = minChunk.x; cx <= maxChunk.x; ++cx) {
        for (std::int32_t cz = minChunk.z; cz <= maxChunk.z; ++cz) {
            ChunkPos const chunkPos{ cx, cz };
            auto it = loadedChunks.find(chunkPos);
            if (it == loadedChunks.end()) {
                continue;
            }
            for (Block const& block : it->second->blocks) {
                if (glm::length(block.pos - worldPos) < radius) {
                    result.push_back(&block);
                }
            }
        }
    }

    return result;
}

bool ChunkManager::removeBlock(Block const* block)
{
    // Removing air blocks is not allowed.
    if (block->type == BlockType::BLOCK_AIR) {
        return false;
    }

    // Get the chunk that the block is in
    ChunkPos chunkPos = getChunkPos(block->pos);
    auto chunkIt      = loadedChunks.find(chunkPos);
    if (chunkIt == loadedChunks.end()) {
        logWarn("Tried to remove block but block was not found in loaded chunks.");
        return false;
    }

    auto& chunkBlocks = chunkIt->second->blocks;
    auto it           = std::find_if(chunkBlocks.begin(), chunkBlocks.end(),
                                     [block](Block const& targetBlock) { return &targetBlock == block; });
    if (it != chunkBlocks.end()) {
        chunkBlocks.erase(it);
    } else {
        logWarn("Tried to remove block but block was not found in the target chunk.");
    }

    return true;
}

ChunkManagerStats ChunkManager::getStats() const
{
    return ChunkManagerStats{ static_cast<std::uint32_t>(loadedChunks.size()),
                              static_cast<std::uint32_t>(allChunks.size()) };
}

} // namespace bg
