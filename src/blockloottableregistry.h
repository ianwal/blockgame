#pragma once

#include "src/block.h"

#include <map>
#include <optional>
#include <string>

namespace bg
{

class BlockLootTableRegistry
{
public:
    std::optional<std::string> getItemDrop(std::string const& blockName) const;

    void load(std::string const& blockName, std::string const& dropItemName);

private:
    std::map<std::string, std::string, std::less<>> lootMap{};
};

} // namespace bg
