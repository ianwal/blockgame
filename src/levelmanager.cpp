#include "src/levelmanager.h"

#include "src/block.h"
#include "src/game.h"

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

void LevelManager::pushOnBlockChanged(ivec3 pos, BlockType oldType, BlockType newType)
{
    for (auto const& fn : blockChangeObservers) {
        fn(pos, oldType, newType);
    }
}

void LevelManager::removeBlock(Block const* block)
{
    Block const oldBlock = *block;
    if (GlobalGame::get().chunkManager.removeBlock(block)) {
        pushOnBlockChanged(oldBlock.pos, oldBlock.type, BlockType::BLOCK_AIR);
    }
}

} // namespace bg
