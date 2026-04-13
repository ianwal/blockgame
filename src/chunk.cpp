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
                // If chunk is not loaded then generate it.
                // logDebug("Generating chunk: {}", to_string(targetChunkPos));

                auto chunk    = std::make_unique<Chunk>();
                chunk->blocks = std::vector<BlockType>(chunkSize * chunkSize * chunkHeight, BlockType::BLOCK_AIR);
                for (int i = 0; i < chunkSize; ++i) {
                    for (int j = 0; j < chunkHeight; ++j) {
                        for (int k = 0; k < chunkSize; ++k) {
                            auto pos                   = getWorldPos(targetChunkPos, ivec3{ i, j, k });
                            float const smoothFactor   = 40.0f;
                            std::int32_t const octaves = 3;
                            auto const surfaceY        = static_cast<std::int32_t>(
                                75 + (perlin.octave2D_11(static_cast<float>(pos.x) / smoothFactor,
                                                                static_cast<float>(pos.z) / smoothFactor, octaves) *
                                      20.0f));
                            auto& blockType = getBlockType(targetChunkPos, *chunk, pos);

                            if (j < surfaceY) {
                                // Below the surface.
                                if (j > (surfaceY - 5)) {
                                    blockType = BlockType::BLOCK_DIRT;
                                } else if (j <= (surfaceY - 5)) {
                                    blockType = BlockType::BLOCK_STONE;
                                } else {
                                    blockType = BlockType::BLOCK_AIR;
                                }
                            } else if (j == surfaceY) {
                                // The surface.
                                blockType = BlockType::BLOCK_GRASS;
                            } else if (j > surfaceY) {
                                // Above the surface.
                                if (surfaceY < 60 && j < 62) {
                                    blockType = BlockType::BLOCK_WATER;
                                } else {
                                    blockType = BlockType::BLOCK_AIR;
                                }
                            }
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

std::vector<Block> ChunkManager::getNearbyBlocks(glm::vec3 const& worldPos, float const radius) const
{
    std::vector<Block> result{};

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
            // TODO: There's no need to iterate over every block. The exact blocks can be calculated beforehand.
            for (std::size_t i = 0; i < it->second->blocks.size(); ++i) {
                Block block{};
                block.pos = getBlockWorldPos(chunkPos, i);
                if (glm::length(tovec3(block.pos) - worldPos) < radius) {
                    block.type = getBlockType(chunkPos, *it->second, block.pos);
                    result.push_back(block);
                }
            }
        }
    }

    return result;
}

std::optional<Block> ChunkManager::getBlock(ivec3 const& blockPos) const
{
    ChunkPos const chunkPos = getChunkPos(blockPos);

    auto chunkIt = loadedChunks.find(chunkPos);
    if (chunkIt == loadedChunks.end()) {
        // Chunk not loaded.
        return std::nullopt;
    }

    Block block{};
    block.pos          = blockPos;
    Chunk const& chunk = *chunkIt->second;
    block.type         = getBlockType(chunkPos, chunk, blockPos);
    return block;
}

bool ChunkManager::placeBlock(Block const& block, BlockType& oldBlockTypeOut)
{
    // Placing an air block is not allowed.
    if (block.type == BlockType::BLOCK_AIR) {
        return false;
    }

    // Get the chunk that the block is in
    ChunkPos const chunkPos = getChunkPos(block.pos);
    auto chunkIt            = loadedChunks.find(chunkPos);
    if (chunkIt == loadedChunks.end()) {
        logWarn("Tried to place a block but block was not found in loaded chunks.");
        return false;
    }

    Chunk& chunk    = *chunkIt->second;
    oldBlockTypeOut = getBlockType(chunkPos, chunk, block.pos);
    // Check type again in case caller didn't pass in the correct block type.
    // Replacing a non-air block is not allowed.
    if (oldBlockTypeOut != BlockType::BLOCK_AIR || oldBlockTypeOut != BlockType::BLOCK_WATER) {
        return false;
    }

    getBlockType(chunkPos, chunk, block.pos) = block.type;
    return true;
}

bool ChunkManager::removeBlock(Block const& block)
{
    // Removing certain blocks is not allowed.
    if (block.type == BlockType::BLOCK_AIR || block.type == BlockType::BLOCK_WATER) {
        return false;
    }

    // Get the chunk that the block is in
    ChunkPos const chunkPos = getChunkPos(block.pos);
    auto chunkIt            = loadedChunks.find(chunkPos);
    if (chunkIt == loadedChunks.end()) {
        logWarn("Tried to place a block but block was not found in loaded chunks.");
        return false;
    }

    Chunk& chunk = *chunkIt->second;
    // Check type in case caller didn't pass in the correct block type.
    // Removing certain blocks is not allowed.
    auto const oldBlockType = getBlockType(chunkPos, chunk, block.pos);
    if (oldBlockType == BlockType::BLOCK_AIR || oldBlockType == BlockType::BLOCK_WATER) {
        return false;
    }

    getBlockType(chunkPos, chunk, block.pos) = BlockType::BLOCK_AIR;
    return true;
}

ChunkManagerStats ChunkManager::getStats() const
{
    return ChunkManagerStats{ static_cast<std::uint32_t>(loadedChunks.size()),
                              static_cast<std::uint32_t>(allChunks.size()) };
}

} // namespace bg
