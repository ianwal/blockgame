#include "src/blocktagregistry.h"

#include "src/assertr.h"
#include "src/block.h"

#include <algorithm>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

namespace bg
{

std::vector<std::string> const& BlockTagRegistry::getTags(std::string const& blockName) const
{
    if (!tagsMap.contains(blockName)) {
        static std::vector<std::string> noTagsSentinel{};
        return noTagsSentinel;
    }

    return tagsMap.at(blockName);
}

bool BlockTagRegistry::hasTag(std::string const& blockName, std::string const& tag) const
{
    if (!tagsMap.contains(blockName)) {
        return false;
    }

    auto const& existingTags = tagsMap.at(blockName);
    return std::find(existingTags.begin(), existingTags.end(), tag) != existingTags.end();
}

void BlockTagRegistry::addTags(std::string const& blockName, std::vector<std::string> tags)
{
    if (!tagsMap.contains(blockName)) {
        tagsMap[blockName] = {};
    }

    auto& existingTags = tagsMap.at(blockName);
    existingTags.insert(existingTags.end(), tags.begin(), tags.end());
}

} // namespace bg
