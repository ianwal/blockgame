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

bool LevelManager::placeBlock(Block const& block)
{
    BlockType oldBlockType{};
    if (GlobalGame::get().chunkManager.placeBlock(block, oldBlockType)) {
        pushOnBlockChanged(block.pos, oldBlockType, block.type);
        return true;
    }
    return false;
}

bool LevelManager::removeBlock(Block const& block)
{
    if (GlobalGame::get().chunkManager.removeBlock(block)) {
        pushOnBlockChanged(block.pos, block.type, BlockType::BLOCK_AIR);
        return true;
    }
    return false;
}

} // namespace bg
