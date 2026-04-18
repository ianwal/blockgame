#include "src/blockloottableregistry.h"

#include "src/assertr.h"

#include <optional>
#include <string>

namespace bg
{

std::optional<std::string> BlockLootTableRegistry::getItemDrop(std::string const& blockName) const
{
    if (!lootMap.contains(blockName)) {
        return std::nullopt;
    }

    return lootMap.at(blockName);
}

void BlockLootTableRegistry::load(std::string const& blockName, std::string const& dropItemName)
{
    assertr(!lootMap.contains(blockName), blockName);
    lootMap[blockName] = dropItemName;
}

} // namespace bg
