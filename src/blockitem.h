#pragma once

#include "src/block.h"
#include "src/item.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace bg
{

/// Get the block type that corresponds with the item type.
///
/// Only placeable items have a block type.
std::optional<BlockType> toBlockType(ItemType itemType);

/// Get the item type that corresponds with the block type.
std::optional<ItemType> toItemType(BlockType blockType);

class BlockItemRegistry
{
public:
    std::optional<std::string> getBlockName(std::string const& itemName);

    std::optional<std::string> getItemName(std::string const& blockName);

    /// Load the item data.
    void load(std::string const& blockName, std::string const& itemName);

private:
    std::map<std::string, std::string, std::less<>> blockItemMap{};
    std::map<std::string, std::string, std::less<>> itemBlockMap{};
};

} // namespace bg
