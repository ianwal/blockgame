#include "src/levelmanager.h"

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

void LevelManager::registerOnBlockChanged(OnBlockChangedObserverFn fn)
{
    blockChangeObservers.push_back(fn);
}

void LevelManager::pushOnBlockChanged(glm::vec3 pos, BlockType oldType, BlockType newType)
{
    for (auto const& fn : blockChangeObservers) {
        fn(pos, oldType, newType);
    }
}

} // namespace bg
