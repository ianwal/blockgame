#pragma once

#include "src/block.h"

#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>

namespace bg
{

// Forward declare to avoid a circular header dependency.
class ChunkRenderer;

/// X and Z size of chunks.
inline constexpr std::int32_t chunkSize{ 16 };

/// Y size of chunks.
inline constexpr std::int32_t chunkHeight{ 32 };

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

/// Get the world position of a block from the provided chunk position and block offset from the chunk position.
glm::vec3 getWorldPos(ChunkPos const& chunkPos, glm::vec3 blockOffset);

class ChunkManager
{
public:
    ChunkManager() = default;

    void loadChunks(glm::vec3 const& worldPos, std::int32_t chunkLoadRadius, std::uint64_t seed);

private:
    struct Chunk {
        std::vector<Block> blocks{};
    };

    std::unordered_map<ChunkPos, Chunk> loadedChunks{};

    friend class ChunkRenderer;
};

} // namespace bg
