#include "src/game.h"

#include "src/block.h"
#include "src/camera.h"
#include "src/chunkrenderer.h"
#include "src/logger.h"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <limits>
#include <utility>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace bg
{

void test_fn(glm::vec3 pos, BlockType oldType, BlockType newType)
{
    logInfo("Block broken.");
}

GameContext::GameContext()
{
    levelManager.registerOnBlockChanged(test_fn);
    levelManager.registerOnBlockChanged(
        [](glm::vec3 pos, BlockType oldType, BlockType newType) { ChunkRenderer::markChunkDirty(getChunkPos(pos)); });
    // Move the player
    player.updatePos(glm::vec3{ 0, 55, 0 });
}

void handleSwingAction(GameContext& game)
{
    // Use first person camera because swinging is done in first person.
    auto& camera = game.player.fpCamera;

    glm::vec3 normalizedCameraFront = glm::normalize(camera.getFront(1.0));
    constexpr float maxHitRange     = 5.f;

    auto blockCandidates = game.chunkManager.getNearbyBlocks(camera.getPos(1.0), maxHitRange);
    auto hitBlock        = blockCandidates.end();
    float closestT       = std::numeric_limits<float>::max();

    for (auto block_it = blockCandidates.begin(); block_it != blockCandidates.end(); ++block_it) {
        auto const& block = **block_it;
        if (block.type == BlockType::BLOCK_AIR) {
            continue;
        }

        // Optimization: This skips checking the logic if the block is beyond the max hitrange.
        if (glm::distance(camera.getPos(1.0), block.pos) > (maxHitRange + 1.f)) {
            continue;
        }

        for (float t = 0; t < maxHitRange; t += 0.1f) {
            glm::vec3 ray = camera.getPos(1.0) + (normalizedCameraFront * t);
            if (isPointInCube(ray, block.pos)) {
                if (t < closestT) {
                    closestT = t;
                    hitBlock = block_it;
                }
                break;
            }
        }
    }

    if (hitBlock != blockCandidates.end()) {
        Block const oldBlock = **hitBlock;
        game.chunkManager.removeBlock(*hitBlock);
        game.levelManager.pushOnBlockChanged(oldBlock.pos, oldBlock.type, BlockType::BLOCK_AIR);
    }
}

void handleMoveAction(GameContext& game, MoveAction const& action)
{
    // logDebug("Tick {} Move Dir: {}", game.tick, std::to_underlying(action.direction));

    float const moveSpeed = (0.2 * game.player.getMoveSpeedMultiplier()) * (game.fastNoClipSpeed ? 2.0f : 1.0f);

    // Use first person camera because moving is done relative to the first person camera.
    auto& currentCamera = game.player.fpCamera;
    auto& player        = game.player;

    if (game.noclip) {
        auto moveLeft = [&player, &currentCamera, &moveSpeed]() {
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
        }

    } else {
        // TODO
    }
}

} // namespace bg
