#pragma once

#include "src/assertr.h"
#include "src/block.h"

#include <array>
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
    WOOD_STICK,
    WOODEN_PICKAXE,
    WOODEN_AXE,
    WOODEN_SHOVEL,
    STONE_PICKAXE,
    STONE_AXE,
    STONE_SHOVEL,
};

struct ItemStack {
    ItemType item{};
    std::uint32_t amount{};
};

inline constexpr auto pickaxes = std::to_array({ ItemType::WOODEN_PICKAXE, ItemType::STONE_PICKAXE });
inline constexpr auto axes     = std::to_array({ ItemType::WOODEN_AXE, ItemType::STONE_AXE });
inline constexpr auto shovels  = std::to_array({ ItemType::WOODEN_SHOVEL, ItemType::STONE_SHOVEL });

inline constexpr auto woodTools =
    std::to_array({ ItemType::WOODEN_PICKAXE, ItemType::WOODEN_AXE, ItemType::WOODEN_SHOVEL });
inline constexpr auto stoneTools =
    std::to_array({ ItemType::STONE_PICKAXE, ItemType::STONE_AXE, ItemType::STONE_SHOVEL });

std::string to_string(ItemType itemType);

GLuint getItemTexture(ItemType itemType);

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
