#include "src/chunk.h"

#include "src/assertr.h"
#include "src/block.h"
#include "src/lib/PerlinNoise.hpp"
#include "src/logger.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <format>
#include <map>
#include <optional>
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

ChunkPos getChunkPos(ivec3 const& worldPos)
{
    return getChunkPos(glm::vec3{ worldPos.x, worldPos.y, worldPos.z });
}

glm::vec3 getWorldPos(ChunkPos const& chunkPos, glm::vec3 blockOffset)
{
    return glm::vec3{ (chunkPos.x * chunkSize) + blockOffset.x, blockOffset.y,
                      (chunkPos.z * chunkSize) + blockOffset.z };
}

ChunkUpdates ChunkManager::updateChunks(glm::vec3 const& worldPos, std::int32_t const chunkLoadRadius,
                                        std::uint64_t const seed)
{
    static const siv::PerlinNoise::seed_type perlinSeed = seed;
    static siv::PerlinNoise perlin{ perlinSeed };

    // Get the chunks that are currently loaded before any updates.
    std::vector<ChunkPos> prevLoadedChunks{};
    prevLoadedChunks.reserve(loadedChunks.size());
    std::transform(loadedChunks.begin(), loadedChunks.end(), std::back_inserter(prevLoadedChunks),
                   [](const auto& pair) { return pair.first; });

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

                // If chunk is not loaded, generate it.
                auto chunk = std::make_unique<Chunk>();
                for (int i = 0; i < chunkSize; ++i) {
                    for (int j = 0; j < chunkHeight; ++j) {
                        for (int k = 0; k < chunkSize; ++k) {
                            Block block{};
                            block.pos                  = getWorldPos(targetChunkPos, ivec3{ i, j, k });
                            float const smoothFactor   = 40.0f;
                            std::int32_t const octaves = 3;
                            auto const surfaceY        = static_cast<std::int32_t>(
                                75 + (perlin.octave2D_11(static_cast<float>(block.pos.x) / smoothFactor,
                                                                static_cast<float>(block.pos.z) / smoothFactor, octaves) *
                                      20.0f));

                            if (block.pos.y < surfaceY) {
                                // Below the surface.
                                if (block.pos.y > (surfaceY - 5)) {
                                    block.type = BlockType::BLOCK_DIRT;
                                } else if (block.pos.y <= (surfaceY - 5)) {
                                    block.type = BlockType::BLOCK_COBBLE;
                                } else {
                                    block.type = BlockType::BLOCK_AIR;
                                }
                            } else if (block.pos.y == surfaceY) {
                                // The surface.
                                block.type = BlockType::BLOCK_GRASS;
                            } else {
                                // Above the surface.
                                block.type = BlockType::BLOCK_AIR;
                            }
                            chunk->blocks.push_back(block);
                        }
                    }
                }

                assertr(chunk->blocks.size() == (chunkSize * chunkHeight * chunkSize),
                        std::format("Chunk had {} blocks but should have {} blocks", chunk->blocks.size(),
                                    (chunkSize * chunkHeight * chunkSize)));
                allChunks.insert({ targetChunkPos, std::move(chunk) });
            }
        }
    }

    // Get the chunks that are currently loaded after all updates.
    std::vector<ChunkPos> currLoadedChunks{};
    currLoadedChunks.reserve(loadedChunks.size());
    std::transform(loadedChunks.begin(), loadedChunks.end(), std::back_inserter(currLoadedChunks),
                   [](const auto& pair) { return pair.first; });

    ChunkUpdates chunkUpdates{};
    for (auto const& currLoadedChunk : currLoadedChunks) {
        if (!std::ranges::contains(prevLoadedChunks, currLoadedChunk)) {
            chunkUpdates.newlyLoadedChunks.push_back(currLoadedChunk);
        }
    }

    for (auto const& prevLoadedChunk : prevLoadedChunks) {
        if (!std::ranges::contains(currLoadedChunks, prevLoadedChunk)) {
            chunkUpdates.newlyUnloadedChunks.push_back(prevLoadedChunk);
        }
    }

    return chunkUpdates;
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
                if (glm::length(tovec3(block.pos) - worldPos) < radius) {
                    result.push_back(&block);
                }
            }
        }
    }

    return result;
}

std::optional<Block> ChunkManager::getBlock(ivec3 const& blockPos) const
{
    Block result{};

    ChunkPos const chunk = getChunkPos(blockPos);

    auto it = loadedChunks.find(chunk);
    if (it == loadedChunks.end()) {
        // Chunk not loaded.
        return std::nullopt;
    }

    for (Block const& block : it->second->blocks) {
        if (block.pos == blockPos) {
            return block;
        }
    }

    return std::nullopt;
}

bool ChunkManager::placeBlock(Block const& block, BlockType& oldBlockTypeOut)
{
    // Placing an air block is not allowed.
    if (block.type == BlockType::BLOCK_AIR) {
        return false;
    }

    // Get the chunk that the block is in
    ChunkPos chunkPos = getChunkPos(block.pos);
    auto chunkIt      = loadedChunks.find(chunkPos);
    if (chunkIt == loadedChunks.end()) {
        logWarn("Tried to place a block but block was not found in loaded chunks.");
        return false;
    }

    auto& chunkBlocks = chunkIt->second->blocks;
    auto it           = std::find_if(chunkBlocks.begin(), chunkBlocks.end(),
                                     [block](Block const& targetBlock) { return targetBlock.pos == block.pos; });
    if (it == chunkBlocks.end()) {
        logWarn("Tried to place a block that doesn't exist or isn't currently loaded.");
        return false;
    }

    // Check type again in case caller didn't pass in the correct block type.
    // Replacing a non-air block is not allowed.
    if (it->type != BlockType::BLOCK_AIR) {
        return false;
    }

    it->type = block.type;
    return true;
}

bool ChunkManager::removeBlock(Block const& block)
{
    // Removing air blocks is not allowed.
    if (block.type == BlockType::BLOCK_AIR) {
        return false;
    }

    // Get the chunk that the block is in
    ChunkPos chunkPos = getChunkPos(block.pos);
    auto chunkIt      = loadedChunks.find(chunkPos);
    if (chunkIt == loadedChunks.end()) {
        logWarn("Tried to remove block but block was not found in loaded chunks.");
        return false;
    }

    auto& chunkBlocks = chunkIt->second->blocks;
    auto it           = std::find_if(chunkBlocks.begin(), chunkBlocks.end(),
                                     [block](Block const& targetBlock) { return targetBlock.pos == block.pos; });
    if (it == chunkBlocks.end()) {
        logWarn("Tried to remove block that doesn't exist or isn't currently loaded.");
        return false;
    }

    // Check type again in case caller didn't pass in the correct block type.
    // Removing air blocks is not allowed.
    if (it->type == BlockType::BLOCK_AIR) {
        return false;
    }

    it->type = BlockType::BLOCK_AIR;
    return true;
}

ChunkManagerStats ChunkManager::getStats() const
{
    return ChunkManagerStats{ static_cast<std::uint32_t>(loadedChunks.size()),
                              static_cast<std::uint32_t>(allChunks.size()) };
}

} // namespace bg
