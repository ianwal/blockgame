#include "src/game.h"

#include "src/assertr.h"
#include "src/audio.h"
#include "src/block.h"
#include "src/camera.h"
#include "src/chunkrenderer.h"
#include "src/logger.h"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <limits>
#include <optional>
#include <utility>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace bg
{

namespace
{

constexpr float maxHitRange = 5.0f;

struct HitResult {
    float t{};          /// Distance along ray
    glm::vec3 normal{}; /// Face normal
};

constexpr std::optional<HitResult> rayIntersectsCube(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 cubePos)
{
    // get min/max of the plane because the cube pos is in the center of the cube
    glm::vec3 min = cubePos - 0.5f;
    glm::vec3 max = cubePos + 0.5f;

    // invert here to divide once to avoid dividing multiple times below
    glm::vec3 invDir = 1.0f / rayDir;

    // solve the vector/plane intersection problem
    glm::vec3 t0 = (min - rayOrigin) * invDir; // entry t per axis
    glm::vec3 t1 = (max - rayOrigin) * invDir; // exit  t per axis

    glm::vec3 tMin = glm::min(t0, t1);
    glm::vec3 tMax = glm::max(t0, t1);

    float tEntry = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
    float tExit  = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

    if (tExit < 0.0f || tEntry > tExit) {
        return std::nullopt; // no intersection
    }

    // Determine which face was hit (largest tMin component = entry face)
    glm::vec3 normal(0.0f);
    if (tMin.x > tMin.y && tMin.x > tMin.z) {
        normal.x = (rayDir.x < 0) ? 1.0f : -1.0f;
    } else if (tMin.y > tMin.z) {
        normal.y = (rayDir.y < 0) ? 1.0f : -1.0f;
    } else {
        normal.z = (rayDir.z < 0) ? 1.0f : -1.0f;
    }

    return HitResult{ tEntry, normal };
}

} // namespace

GameContext::GameContext()
{
    levelManager.registerOnBlockChanged([](ivec3 pos, BlockType oldType, BlockType newType) {
        static_cast<void>(oldType);
        static_cast<void>(newType);
        ChunkRenderer::markChunkDirty(getChunkPos(pos));
    });

    // Move the player
    player.updatePos(glm::vec3{ 0, 55, 0 });

    // Add some items
    playerInventory.addItem(ItemStack{ ItemType::DIRT_BLOCK, 10 });
    playerInventory.addItem(ItemStack{ ItemType::LUCKY_BLOCK, 10 });
}

std::optional<Block> getBlockPlayerIsLookingAt(ChunkManager const& chunkManager, Player const& player)
{
    // Use first person camera because looking is done in first person.
    auto& camera = player.fpCamera;

    glm::vec3 normalizedCameraFront = glm::normalize(camera.getFront(1.0));

    auto blockCandidates = chunkManager.getNearbyBlocks(camera.getPos(1.0), maxHitRange);
    auto hitBlock        = blockCandidates.end();
    float closestT       = std::numeric_limits<float>::max();

    glm::vec3 rayOrigin = camera.getPos(1.0);
    glm::vec3 rayDir    = glm::normalize(camera.getFront(1.0));

    for (auto block_it = blockCandidates.begin(); block_it != blockCandidates.end(); ++block_it) {
        auto const& block = **block_it;
        if (block.type == BlockType::BLOCK_AIR) {
            continue;
        }

        // Optimization: This skips checking the logic if the block is beyond the max hitrange.
        if (glm::distance(camera.getPos(1.0), tovec3(block.pos)) > (maxHitRange + 1.f)) {
            continue;
        }

        auto hit = rayIntersectsCube(rayOrigin, rayDir, tovec3(block.pos));
        if (hit && hit->t >= 0.0f && hit->t < maxHitRange && hit->t < closestT) {
            closestT = hit->t;
            hitBlock = block_it;
        }
    }

    if (hitBlock != blockCandidates.end()) {
        return **hitBlock;
    }
    return std::nullopt;
}

std::optional<ivec3> getBlockPosToPlaceAt(ChunkManager const& chunkManager, Player const& player)
{
    // Use first person camera because looking is done in first person.
    auto& camera = player.fpCamera;

    glm::vec3 normalizedCameraFront = glm::normalize(camera.getFront(1.0));

    auto blockCandidates = chunkManager.getNearbyBlocks(camera.getPos(1.0), maxHitRange);
    auto hitBlock        = blockCandidates.end();
    float closestT       = std::numeric_limits<float>::max();
    glm::vec3 normal{};

    glm::vec3 rayOrigin = camera.getPos(1.0);
    glm::vec3 rayDir    = glm::normalize(camera.getFront(1.0));

    for (auto block_it = blockCandidates.begin(); block_it != blockCandidates.end(); ++block_it) {
        auto const& block = **block_it;
        if (block.type == BlockType::BLOCK_AIR) {
            continue;
        }

        // Optimization: This skips checking the logic if the block is beyond the max hitrange.
        if (glm::distance(camera.getPos(1.0), tovec3(block.pos)) > (maxHitRange + 1.f)) {
            continue;
        }

        auto hit = rayIntersectsCube(rayOrigin, rayDir, tovec3(block.pos));
        if (hit && (hit->t >= 0.0f) && (hit->t < maxHitRange) && (hit->t < closestT)) {
            closestT = hit->t;
            hitBlock = block_it;
            normal   = hit->normal;
        }
    }

    if (hitBlock == blockCandidates.end()) {
        return std::nullopt;
    }

    auto pos = (*hitBlock)->pos;
    // Note: This assumes block size is 1.0 because the normal is 1.0
    return toBlockPos(glm::vec3{ pos.x, pos.y, pos.z } + normal);
}

void handleSwingAction(GameContext& game)
{
    std::optional<Block> blockResult = getBlockPlayerIsLookingAt(game.chunkManager, game.player);
    if (!blockResult) {
        return;
    }

    Block const block = blockResult.value();
    game.levelManager.removeBlock(block);
    playSound2d(getBlockBreakSoundName(block.type));
}

void handleMoveAction(GameContext& game, MoveAction const& action)
{
    // logDebug("Tick {} Move Dir: {}", game.tick, std::to_underlying(action.direction));

    // Use first person camera because moving is done relative to the first person camera.
    auto& currentCamera = game.player.fpCamera;
    auto& player        = game.player;

    if (player.isFlying()) {
        float const moveSpeed = (0.2 * player.getMoveSpeedMultiplier()) * (game.fastNoClipSpeed ? 2.0f : 1.0f);
        auto moveLeft         = [&player, &currentCamera, &moveSpeed]() {
            // Add velocity in the direction.
            glm::vec3 const dir = glm::normalize(glm::cross(currentCamera.getFront(1.0), currentCamera.getUp()));
            glm::vec3 const newVelocity = (player.getVelocity() - (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveRight = [&player, &currentCamera, &moveSpeed]() {
            // Add velocity in the direction.
            glm::vec3 const dir = glm::normalize(glm::cross(currentCamera.getFront(1.0), currentCamera.getUp()));
            glm::vec3 const newVelocity = (player.getVelocity() + (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveForward = [&player, &currentCamera, &moveSpeed]() {
            // Add velocity in the direction.
            glm::vec3 const dir         = currentCamera.getFront(1.0);
            glm::vec3 const newVelocity = (player.getVelocity() + (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveBackward = [&player, &currentCamera, &moveSpeed]() {
            glm::vec3 const dir         = currentCamera.getFront(1.0);
            glm::vec3 const newVelocity = (player.getVelocity() - (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveUp = [&player, &currentCamera, &moveSpeed]() {
            glm::vec3 const dir         = glm::vec3{ 0, 1, 0 };
            glm::vec3 const newVelocity = (player.getVelocity() + (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveDown = [&player, &currentCamera, &moveSpeed]() {
            glm::vec3 const dir         = glm::vec3{ 0, -1, 0 };
            glm::vec3 const newVelocity = (player.getVelocity() + (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        switch (action.direction) {
            case MoveAction::Direction::LEFT: {
                moveLeft();
            } break;
            case MoveAction::Direction::RIGHT: {
                moveRight();
            } break;
            case MoveAction::Direction::UP: {
                moveUp();
            } break;
            case MoveAction::Direction::DOWN: {
                moveDown();
            } break;
            case MoveAction::Direction::FORWARD: {
                moveForward();
            } break;
            case MoveAction::Direction::BACKWARD: {
                moveBackward();
            } break;
            case MoveAction::Direction::NONE: {
                // Do nothing.
            } break;
        }
    } else {
        // TODO: The non-grounded movement is janky and needs to be fixed.
        float const moveSpeed = (0.1 * player.getMoveSpeedMultiplier() * (player.isGrounded() ? 1.0f : 0.8f));
        auto moveLeft         = [&player, &currentCamera, &moveSpeed]() {
            // Add velocity in the direction.
            glm::vec3 const dir = glm::normalize(glm::cross(currentCamera.getFront(1.0), currentCamera.getUp()));
            glm::vec3 const newVelocity = (player.getVelocity() - (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveRight = [&player, &currentCamera, &moveSpeed]() {
            // Add velocity in the direction.
            glm::vec3 const dir = glm::normalize(glm::cross(currentCamera.getFront(1.0), currentCamera.getUp()));
            glm::vec3 const newVelocity = (player.getVelocity() + (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveForward = [&player, &currentCamera, &moveSpeed]() {
            // Add velocity in the direction.
            glm::vec3 const dir = glm::normalize(
                glm::cross(glm::cross(currentCamera.getFront(1.0), currentCamera.getUp()), currentCamera.getUp()));
            glm::vec3 const newVelocity = (player.getVelocity() - (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveBackward = [&player, &currentCamera, &moveSpeed]() {
            glm::vec3 const dir = glm::normalize(
                glm::cross(glm::cross(currentCamera.getFront(1.0), currentCamera.getUp()), currentCamera.getUp()));
            glm::vec3 const newVelocity = (player.getVelocity() + (moveSpeed * dir));
            player.setVelocity(newVelocity);
        };

        auto moveUp = [&player, &currentCamera, &moveSpeed]() {
            if (player.isGrounded()) {
                float jumpMultiplier        = 0.4;
                glm::vec3 const dir         = glm::vec3{ 0, 1, 0 };
                glm::vec3 const newVelocity = (player.getVelocity() + (jumpMultiplier * dir));
                player.setVelocity(newVelocity);
            }
        };

        switch (action.direction) {
            case MoveAction::Direction::LEFT: {
                moveLeft();
            } break;
            case MoveAction::Direction::RIGHT: {
                moveRight();
            } break;
            case MoveAction::Direction::UP: {
                moveUp();
            } break;
            case MoveAction::Direction::DOWN: {
                // Do nothing.
            } break;
            case MoveAction::Direction::FORWARD: {
                moveForward();
            } break;
            case MoveAction::Direction::BACKWARD: {
                moveBackward();
            } break;
            case MoveAction::Direction::NONE: {
                // Do nothing.
            } break;
        }
    }
}

void handlePlaceAction(GameContext& game)
{
    std::optional<ivec3> blockPosResult = getBlockPosToPlaceAt(game.chunkManager, game.player);
    if (!blockPosResult) {
        return;
    }

    auto const heldItemResult = game.playerInventory.getCurrentHeldItem();
    if (!heldItemResult.has_value()) {
        return;
    }

    auto blockTypeResult = toBlockType(heldItemResult->item);
    if (!blockTypeResult.has_value()) {
        return;
    }

    Block block{};
    block.pos  = blockPosResult.value();
    block.type = *blockTypeResult;

    // TODO: This should check if any entity with a physical AABB is in the current block, not just the player.
    if (!game.player.getAABB().intersects(getBlockAABB(block)) && game.levelManager.placeBlock(block)) {
        game.playerInventory.removeItem(InventorySlot{ 0, game.playerInventory.getCurrentHotbarSlot() }, 1U);
        game.player.swing();
        playSound2d(getBlockBreakSoundName(block.type));
    }
}

void handleDropItemAction(GameContext& game)
{
    auto heldItemResult = game.playerInventory.getCurrentHeldItem();
    if (heldItemResult.has_value()) {
        game.playerInventory.removeItem(InventorySlot{ 0, game.playerInventory.getCurrentHotbarSlot() }, 1U);
    }
}

void handleChangeHotbarPosAction(GameContext& game, ChangeHotbarPosAction const& action)
{
    game.playerInventory.setCurrentHotbarSlot(action.index);
    // TODO: Add change item animation where the item dips out of view and changes to the new one if the new held item
    // is different.
}

GameContext* GlobalGame::game = nullptr;

GameContext& GlobalGame::get()
{
    assertr(game != nullptr);
    return *game;
}

void GlobalGame::set(GameContext* newGame)
{
    game = newGame;
}

} // namespace bg
