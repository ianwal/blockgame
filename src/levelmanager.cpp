#include "src/levelmanager.h"

#include "src/block.h"
#include "src/game.h"
#include "src/logger.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bg
{

void LevelManager::tick()
{
    for (auto& entity : loadedEntities) {
        entity->tick();
    }
}

void LevelManager::updateChunks(glm::vec3 const& worldPos, std::int32_t chunkLoadRadius, std::uint64_t seed)
{
    ChunkUpdates const chunkUpdates = GlobalGame::get().chunkManager.updateChunks(worldPos, chunkLoadRadius, seed);

    if (!chunkUpdates.newlyLoadedChunks.empty() || !chunkUpdates.newlyUnloadedChunks.empty()) {
        // Note: This load/unload code is pretty badly written and poorly optimized.

        // If there's any chunk updates then load any unloaded entities and unload entities in chunks that were
        // unloaded. Only persistent entities are saved when unloading chunks.
        std::list<std::unique_ptr<IEntity>> entitiesToUnload{};
        for (auto& entity : loadedEntities) {
            assertr(entity != nullptr);
            ChunkPos const entityChunk = getChunkPos(entity->getPos(1.0f));
            if (std::ranges::contains(chunkUpdates.newlyUnloadedChunks, entityChunk)) {
                if (!entity->isPersistent() || entity->isDead()) {
                    // Non-persistent entities and dead entities are deleted when unloaded.
                    entity.reset();
                } else {
                    // entity is safe to move while iterating over loadedEntities because it doesn't modify
                    // loadedEntities.
                    entitiesToUnload.push_back(std::move(entity));
                }
            }
        }

        std::list<std::unique_ptr<IEntity>> entitiesToLoad{};
        for (auto& entity : unloadedPersistentEntities) {
            assertr(entity != nullptr);
            ChunkPos const entityChunk = getChunkPos(entity->getPos(1.0f));
            if (std::ranges::contains(chunkUpdates.newlyLoadedChunks, entityChunk)) {
                // Note: This is safe to move while iterating over loadedEntities because it doesn't modify
                // loadedEntities.
                entitiesToLoad.push_back(std::move(entity));
            }
        }

        // Remove any entities in loadedEntities that are now nullptr due to the unload operation above.
        std::erase_if(loadedEntities, [](auto& e) { return e == nullptr; });

        // Remove any entities in unloadedPersistentEntities that are now nullptr due to the load operation above.
        std::erase_if(unloadedPersistentEntities, [](auto& e) { return e == nullptr; });

        for (auto& entity : entitiesToUnload) {
            unloadedPersistentEntities.push_back(std::move(entity));
        }
        for (auto& entity : entitiesToLoad) {
            loadedEntities.push_back(std::move(entity));
        }
    }
}

void LevelManager::cleanupDeadEntities()
{
    std::vector<IEntity*> deadEntities{};
    for (auto& entity : loadedEntities) {
        if (entity->isDead()) {
            deadEntities.push_back(entity.get());
        }
    }
    for (auto* deadEntity : deadEntities) {
        loadedEntities.erase(std::find_if(loadedEntities.begin(), loadedEntities.end(),
                                          [&deadEntity](auto& entity) { return entity.get() == deadEntity; }));
    }
}

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

void LevelManager::addEntity(std::unique_ptr<IEntity> entity)
{
    entity->setId(nextUniqueEntityId);
    ++nextUniqueEntityId;

    loadedEntities.push_back(std::move(entity));
}

std::vector<std::unique_ptr<IEntity>> const& LevelManager::getEntities()
{
    return loadedEntities;
}

void LevelManager::sortEntities()
{
    std::stable_sort(loadedEntities.begin(), loadedEntities.end(),
                     [](auto const& e1, auto const& e2) { return e1->isOpaque() > e2->isOpaque(); });
}

} // namespace bg
