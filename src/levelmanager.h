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
    using OnBlockChangedObserverFn = std::function<void(glm::vec3 pos, BlockType oldType, BlockType newType)>;

    LevelManager() = default;

    /// Add callback for when a block is changed.
    void registerOnBlockChanged(OnBlockChangedObserverFn fn);

    /// Push that a block changed to all observers. Should only be called by the module that owns the blocks.
    void pushOnBlockChanged(glm::vec3 pos, BlockType oldType, BlockType newType);

private:
    std::vector<OnBlockChangedObserverFn> blockChangeObservers{};
};

} // namespace bg
