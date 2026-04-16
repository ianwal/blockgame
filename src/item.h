#pragma once

#include "src/assertr.h"
#include "src/block.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <utility>

namespace bg
{

enum class ItemType : std::uint16_t {
    STONE_BLOCK = 0,
    COBBLE_BLOCK,
    DIRT_BLOCK,
    GRASS_BLOCK,
    LUCKY_BLOCK,
    OAK_WOOD,
    OAK_LEAF,
    BEDROCK_BLOCK,
    OAKPLANKS,
};

struct ItemStack {
    ItemType item{};
    std::uint32_t amount{};
};

std::string to_string(ItemType itemType);

class ItemRegistry
{
public:
    ItemType getType(std::string const& name);

    std::string getName(ItemType type);

    std::string getPrettyName(ItemType type);

    /// Load the item data.
    void load(std::uint16_t id, std::string const& name, std::string const& prettyName);

private:
    std::map<ItemType, std::string, std::less<>> names{};
    std::map<ItemType, std::string, std::less<>> prettyNames{};
};

} // namespace bg
