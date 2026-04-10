#pragma once

#include "src/block.h"

#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bg
{

class LevelManager
{
public:
    using OnBlockChangedObserverFn = std::function<void(ivec3 pos, BlockType oldType, BlockType newType)>;

    LevelManager() = default;

    /// Add callback for when a block is changed.
    void registerOnBlockChanged(OnBlockChangedObserverFn fn);

    /// Push that a block changed to all observers. Should only be called by the module that owns the blocks.
    void pushOnBlockChanged(ivec3 pos, BlockType oldType, BlockType newType);

    /// Remove a block from the world. Will notify observers.
    void removeBlock(Block const& block);

private:
    std::vector<OnBlockChangedObserverFn> blockChangeObservers{};
};

} // namespace bg
