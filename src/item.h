#pragma once

#include "src/assertr.h"
#include "src/block.h"

#include <cstdint>
#include <optional>
#include <utility>

namespace bg
{

enum class ItemType : std::uint16_t {
    GRASS_BLOCK = 0,
    DIRT_BLOCK,
    LUCKY_BLOCK,
};

constexpr std::string_view to_string(ItemType itemType)
{
    switch (itemType) {
        case ItemType::GRASS_BLOCK:
            return "Grass Block";
        case ItemType::DIRT_BLOCK:
            return "Dirt Block";
        case ItemType::LUCKY_BLOCK:
            return "Lucky Block";
        default: {
            return "Unknown";
        } break;
    }
}

/// Get the block type that corresponds with the item type.
///
/// Only placeable items have a block type.
constexpr std::optional<BlockType> toBlockType(ItemType itemType)
{
    switch (itemType) {
        case ItemType::GRASS_BLOCK:
            return BlockType::BLOCK_GRASS;
        case ItemType::DIRT_BLOCK:
            return BlockType::BLOCK_DIRT;
        case ItemType::LUCKY_BLOCK:
            return BlockType::BLOCK_LUCKY;
        default: {
            return std::nullopt;
        } break;
    }
}

} // namespace bg
