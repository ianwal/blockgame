#include "src/chunk.h"

#include "src/assertr.h"
#include "src/block.h"
#include "src/lib/PerlinNoise.hpp"
#include "src/logger.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <array>
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

bool isBlockInChunk(ChunkPos const& chunkPos, ivec3 const& blockPos)
{
    std::int32_t localX = blockPos.x - (chunkPos.x * chunkSize);
    std::int32_t localZ = blockPos.z - (chunkPos.z * chunkSize);

    return (localX >= 0) && (localX < chunkSize) && (localZ >= 0) && (localZ < chunkSize) && (blockPos.y >= 0) &&
           (blockPos.y < chunkHeight);
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
                std::vector<Block> surfaceBlocks{};
                surfaceBlocks.reserve(chunkSize * chunkSize);
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

                            std::int32_t const sealevel = 70;
                            if (j < surfaceY) {
                                // Below the surface.
                                if (j > (surfaceY - 5)) {
                                    blockType = BlockType::BLOCK_DIRT;
                                } else if (j <= (surfaceY - 5)) {
                                    if (j == 0) {
                                        blockType = BlockType::BLOCK_BEDROCK;
                                    } else {
                                        blockType = BlockType::BLOCK_STONE;
                                    }
                                } else {
                                    blockType = BlockType::BLOCK_AIR;
                                }
                            } else if (j == surfaceY) {
                                // The surface.
                                if (surfaceY >= sealevel) {
                                    blockType = BlockType::BLOCK_GRASS;
                                } else {
                                    blockType = BlockType::BLOCK_DIRT;
                                }
                                surfaceBlocks.push_back(Block{ toBlockPos(pos), blockType });
                            } else if (j > surfaceY) {
                                // Above the surface.
                                if (j <= sealevel) {
                                    blockType = BlockType::BLOCK_WATER;
                                } else {
                                    blockType = BlockType::BLOCK_AIR;
                                }
                            }
                        }
                    }
                }

                auto const cx = static_cast<std::uint64_t>(static_cast<std::int64_t>(targetChunkPos.x));
                auto const cz = static_cast<std::uint64_t>(static_cast<std::int64_t>(targetChunkPos.z));

                std::mt19937_64 chunkLocalRNGGen{
                    (seed + cx * cx * 4987142ULL + cx * 5947611ULL + cz * cz * 4392871ULL + cz * 389711ULL) ^ seed
                };

                // Generate trees
                {
                    std::vector<ivec3> targetTreePositions{};
                    for (auto const& surfaceBlock : surfaceBlocks) {
                        if (surfaceBlock.type == BlockType::BLOCK_GRASS) {
                            targetTreePositions.push_back(surfaceBlock.pos + ivec3{ 0, 1, 0 });
                        }
                    }

                    if (!targetTreePositions.empty()) {
                        std::uniform_int_distribution<> treeCountDist(0, 3);
                        std::uniform_int_distribution<> treeDist(0, targetTreePositions.size() - 1);
                        int const treeCount = treeCountDist(chunkLocalRNGGen);
                        for (int i = 0; i < treeCount; ++i) {
                            if (targetTreePositions.empty()) {
                                break;
                            }
                            ivec3 const treePos = targetTreePositions[treeDist(chunkLocalRNGGen)];

                            static constexpr auto logOffsets = std::to_array({
                                ivec3{ 0, 0, 0 },
                                ivec3{ 0, 1, 0 },
                                ivec3{ 0, 2, 0 },
                                ivec3{ 0, 3, 0 },
                                ivec3{ 0, 4, 0 },
                            });
                            // clang-format off
                            static constexpr auto leafOffsets = std::to_array({
                                ivec3{ -1, 2, 2 },
                                ivec3{ 0, 2, 2 },
                                ivec3{ 1, 2, 2 },
                                ivec3{ 2, 2, 2 },
                                ivec3{ -1, 3, 2 },
                                ivec3{ 0, 3, 2 },
                                ivec3{ 1, 3, 2 },

                                ivec3{ -2, 2, 1 },
                                ivec3{ -1, 2, 1 },
                                ivec3{ 0, 2, 1 },
                                ivec3{ 1, 2, 1 },
                                ivec3{ 2, 2, 1 },
                                ivec3{ -2, 3, 1 },
                                ivec3{ -1, 3, 1 },
                                ivec3{ 0, 3, 1 },
                                ivec3{ 1, 3, 1 },
                                ivec3{ 2, 3, 1 },
                                ivec3{ 0, 4, 1 },
                                ivec3{ -1, 4, 1 },
                                ivec3{ 0, 5, 1 },

                                ivec3{ -2, 2, 0 },
                                ivec3{ -1, 2, 0 },
                                ivec3{ 1, 2, 0 },
                                ivec3{ 2, 2, 0 },
                                ivec3{ -2, 3, 0 },
                                ivec3{ -1, 3, 0 },
                                ivec3{ 1, 3, 0 },
                                ivec3{ 2, 3, 0 },
                                ivec3{ -1, 4, 0 },
                                ivec3{ -1, 5, 0 },
                                ivec3{ 0, 5, 0 },
                                ivec3{ 1, 5, 0 },
                                ivec3{ 1, 4, 0 },

                                ivec3{ -2, 2, -1 },
                                ivec3{ -1, 2, -1 },
                                ivec3{ 0, 2, -1 },
                                ivec3{ 1, 2, -1 },
                                ivec3{ 2, 2, -1 },
                                ivec3{ -2, 3, -1 },
                                ivec3{ -1, 3, -1 },
                                ivec3{ 0, 3, -1 },
                                ivec3{ 1, 3, -1 },
                                ivec3{ 2, 3, -1 },
                                ivec3{ 0, 4, -1 },
                                ivec3{ 0, 5, -1 },

                                ivec3{ -1, 2, -2 },
                                ivec3{ 0, 2, -2 },
                                ivec3{ 1, 2, -2 },
                                ivec3{ -2, 3, -2 },
                                ivec3{ -1, 3, -2 },
                                ivec3{ 0, 3, -2 },
                                ivec3{ 1, 3, -2 },
                                ivec3{ 2, 3, -2 },
                            });
                            // clang-format on
                            bool canGenTree = true;
                            for (auto const& offset : logOffsets) {
                                if (!isBlockInChunk(targetChunkPos, treePos + offset)) {
                                    canGenTree = false;
                                    break;
                                }
                            }
                            for (auto const& offset : leafOffsets) {
                                if (!isBlockInChunk(targetChunkPos, treePos + offset)) {
                                    canGenTree = false;
                                    break;
                                }
                            }

                            if (canGenTree) {
                                for (auto const& offset : logOffsets) {
                                    getBlockType(targetChunkPos, *chunk, treePos + offset) = BlockType::BLOCK_OAK;
                                }
                                for (auto const& offset : leafOffsets) {
                                    getBlockType(targetChunkPos, *chunk, treePos + offset) = BlockType::BLOCK_OAKLEAF;
                                }
                                std::erase(targetTreePositions, treePos);
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
    if (oldBlockTypeOut != BlockType::BLOCK_AIR && oldBlockTypeOut != BlockType::BLOCK_WATER) {
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
    if (oldBlockType == BlockType::BLOCK_AIR && oldBlockType == BlockType::BLOCK_WATER) {
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
