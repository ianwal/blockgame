#include "src/blockitem.h"

#include "src/app_context.h"
#include "src/assertr.h"
#include "src/block.h"
#include "src/item.h"

#include <format>
#include <optional>
#include <string>
#include <string_view>

namespace bg
{

std::optional<BlockType> toBlockType(ItemType itemType)
{
    auto val = App::get().blockItemRegistry.getBlockName(App::get().itemRegistry.getName(itemType));
    if (val.has_value()) {
        return App::get().blockRegistry.getType(val.value());
    }
    return std::nullopt;
}

std::optional<ItemType> toItemType(BlockType blockType)
{
    auto val = App::get().blockItemRegistry.getItemName(App::get().blockRegistry.getName(blockType));
    if (val.has_value()) {
        return App::get().itemRegistry.getType(val.value());
    }
    return std::nullopt;
}

std::optional<std::string> BlockItemRegistry::getBlockName(std::string const& itemName)
{
    if (itemBlockMap.contains(itemName)) {
        return itemBlockMap[itemName];
    }
    return std::nullopt;
}

std::optional<std::string> BlockItemRegistry::getItemName(std::string const& blockName)
{
    if (blockItemMap.contains(blockName)) {
        return blockItemMap[blockName];
    }
    return std::nullopt;
}

void BlockItemRegistry::load(std::string const& blockName, std::string const& itemName)
{
    assertr(!blockItemMap.contains(blockName), blockName);
    assertr(!itemBlockMap.contains(itemName), itemName);

    blockItemMap[blockName] = itemName;
    itemBlockMap[itemName]  = blockName;
}

} // namespace bg
