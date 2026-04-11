#pragma once

#include "src/block.h"
#include "src/entity.h"

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

/// Manages all entities and chunks in the game.
class LevelManager
{
public:
    using OnBlockChangedObserverFn = std::function<void(ivec3 pos, BlockType oldType, BlockType newType)>;

    LevelManager() = default;

    void tick();

    void cleanupDeadEntities();

    /// Add callback for when a block is changed.
    void registerOnBlockChanged(OnBlockChangedObserverFn fn);

    /// Push that a block changed to all observers. Should only be called by the module that owns the blocks.
    void pushOnBlockChanged(ivec3 pos, BlockType oldType, BlockType newType);

    /// Place a block in the world. Will notify observers.
    bool placeBlock(Block const& block);

    /// Remove a block from the world. Will notify observers.
    bool removeBlock(Block const& block);

    void addEntity(std::unique_ptr<IEntity> entity);

    std::vector<std::unique_ptr<IEntity>> const& getEntities();

private:
    std::vector<OnBlockChangedObserverFn> blockChangeObservers{};

    std::vector<std::unique_ptr<IEntity>> loadedEntities{};

    EntityId nextUniqueEntityId{};
};

} // namespace bg
