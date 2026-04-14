#pragma once

#include "src/block.h"

#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bg
{

// Forward declare to avoid a circular header dependency.
class ChunkRenderer;

/// X and Z size of chunks.
inline constexpr std::int32_t chunkSize{ 16 };

/// Y size of chunks.
inline constexpr std::int32_t chunkHeight{ 128 };

/// Position of the chunk. The chunk goes all the way from the world floor to ceiling, so y is not needed.
/// The {x, z} combination is the unique identifier of the chunk in the world.
struct ChunkPos {
    std::int32_t x{};
    std::int32_t z{};

    bool operator==(ChunkPos const&) const = default;
    bool operator!=(ChunkPos const&) const = default;
};

} // namespace bg

template <>
struct std::hash<bg::ChunkPos> {
    std::size_t operator()(bg::ChunkPos const& p) const noexcept
    {
        static_assert(sizeof(std::size_t) >= sizeof(std::uint64_t));
        std::uint64_t packed = (std::uint64_t) (std::uint32_t) p.x << 32U | (std::uint32_t) p.z;
        return std::hash<std::uint64_t>{}(packed);
    }
};

namespace bg
{

std::string to_string(ChunkPos const& chunkPos);

/// Get the position of the chunk that would contain the block in the given world position.
///
/// This is a lossy operation - the world pos can't be recovered once converting to the chunk pos.
ChunkPos getChunkPos(glm::vec3 const& worldPos);

/// Get the position of the chunk that would contain the block in the given world position.
///
/// This is a lossy operation - the world pos can't be recovered once converting to the chunk pos.
ChunkPos getChunkPos(ivec3 const& worldPos);

/// Get the world position of a block from the provided chunk position and block offset from the chunk position.
glm::vec3 getWorldPos(ChunkPos const& chunkPos, glm::vec3 blockOffset);

/// Check if a block at the given world position is within the target chunk.
bool isBlockInChunk(ChunkPos const& chunkPos, ivec3 const& blockPos);

struct ChunkManagerStats {
    std::uint32_t loadedChunkCount{};
    std::uint32_t worldChunkCount{};
};

struct ChunkUpdates {
    std::vector<ChunkPos> newlyUnloadedChunks{};
    std::vector<ChunkPos> newlyLoadedChunks{};
};

class ChunkManager
{
public:
    ChunkManager() = default;

    ChunkUpdates updateChunks(glm::vec3 const& worldPos, std::int32_t chunkLoadRadius, std::uint64_t seed);

    std::vector<Block> getNearbyBlocks(glm::vec3 const& worldPos, float radius) const;

    /// Return the block if the chunk that holds the block is currently loaded.
    std::optional<Block> getBlock(ivec3 const& blockPos) const;

    bool placeBlock(Block const& block, BlockType& oldBlockTypeOut);

    bool removeBlock(Block const& block);

    ChunkManagerStats getStats() const;

private:
    struct Chunk {
        /// Blocks as a 1D array.
        ///
        /// The index of the block corresponds with the block position.
        std::vector<BlockType> blocks{};
    };

    /// Get the type of the target block.
    [[nodiscard]] static BlockType& getBlockType(ChunkPos chunkPos, Chunk& chunk, ivec3 const& blockWorldPos)
    {
        std::int32_t localX = blockWorldPos.x - (chunkPos.x * chunkSize);
        std::int32_t localY = blockWorldPos.y;
        std::int32_t localZ = blockWorldPos.z - (chunkPos.z * chunkSize);
        return chunk.blocks[localX + chunkSize * (localZ + chunkSize * localY)];
    }

    /// Get the type of the target block.
    [[nodiscard]] static BlockType getBlockType(ChunkPos chunkPos, Chunk const& chunk, ivec3 const& blockWorldPos)
    {
        std::int32_t localX = blockWorldPos.x - (chunkPos.x * chunkSize);
        std::int32_t localY = blockWorldPos.y;
        std::int32_t localZ = blockWorldPos.z - (chunkPos.z * chunkSize);
        return chunk.blocks.at(localX + chunkSize * (localZ + chunkSize * localY));
    }

    [[nodiscard]] static ivec3 getBlockWorldPos(ChunkPos chunkPos, std::size_t index)
    {
        int localY = index / (chunkSize * chunkSize);
        int localZ = (index / chunkSize) % chunkSize;
        int localX = index % chunkSize;

        return { localX + chunkPos.x * chunkSize, localY, localZ + chunkPos.z * chunkSize };
    }

    std::unordered_map<ChunkPos, Chunk*>
        loadedChunks{}; // TODO: Make pointer to the iterator in allChunks and refresh every tick?
    std::unordered_map<ChunkPos, std::unique_ptr<Chunk>> allChunks{};

    friend class ChunkRenderer;
};

} // namespace bg
