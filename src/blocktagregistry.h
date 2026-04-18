#pragma once

#include "src/block.h"

#include <map>
#include <string>
#include <vector>

namespace bg
{

class BlockTagRegistry
{
public:
    std::vector<std::string> const& getTags(std::string const& blockName) const;

    bool hasTag(std::string const& blockName, std::string const& tag) const;

    void addTags(std::string const& blockName, std::vector<std::string> tags);

private:
    // TODO: Deduplicate tags across blocks to reduce memory usage per block type?
    std::map<std::string, std::vector<std::string>, std::less<>> tagsMap{};
};

} // namespace bg
