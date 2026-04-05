#include "src/game.h"

#include "src/block.h"
#include "src/camera.h"
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

GameContext::GameContext()
{
    // Generate some blocks.
    // Move the player
    player.updatePos(glm::vec3{ 0, 68, 0 });
}

void handleSwingAction(GameContext& game)
{
    // TODO: Fix.
    /*
    // Use first person camera because swinging is done in first person.
    auto& camera = game.player.fpCamera;

    glm::vec3 normalizedCameraFront = glm::normalize(camera.getFront(1.0));
    constexpr float maxHitRange     = 5.f;
    constexpr float step            = 0.05f;

    auto hitBlock  = game.blocks.end();
    float closestT = std::numeric_limits<float>::max();

    for (auto block_it = game.blocks.begin(); block_it != game.blocks.end(); ++block_it) {
        auto const& block = *block_it;
        if (block.type == BlockType::BLOCK_AIR) {
            continue;
        }

        // Optimization: This skips checking the logic if the block is beyond the max hitrange.
        if (glm::distance(camera.getPos(1.0), block.pos) > maxHitRange + 1.f) {
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

    if (hitBlock != game.blocks.end()) {
        game.blocks.erase(hitBlock);
    }
    */
}

void handleMoveAction(GameContext& game, MoveAction const& action)
{
    // logDebug("Tick {} Move Dir: {}", game.tick, std::to_underlying(action.direction));

    float const moveSpeed = (1.0 * game.player.getMoveSpeedMultiplier()) * (game.fastNoClipSpeed ? 2.0f : 1.0f);

    // Use first person camera because moving is done relative to the first person camera.
    auto& currentCamera = game.player.fpCamera;
    auto& player        = game.player;

    if (game.noclip) {
        auto moveLeft = [&player, &currentCamera, &moveSpeed]() {
            player.updatePos(player.getPos(1.0) -
                             glm::normalize(glm::cross(currentCamera.getFront(1.0), currentCamera.getUp())) *
                                 moveSpeed);
        };

        auto moveRight = [&player, &currentCamera, &moveSpeed]() {
            player.updatePos(player.getPos(1.0) +
                             glm::normalize(glm::cross(currentCamera.getFront(1.0), currentCamera.getUp())) *
                                 moveSpeed);
        };

        auto moveForward = [&player, &currentCamera, &moveSpeed]() {
            player.updatePos(player.getPos(1.0) + (currentCamera.getFront(1.0) * moveSpeed));
        };

        auto moveBackward = [&player, &currentCamera, &moveSpeed]() {
            player.updatePos(player.getPos(1.0) - (currentCamera.getFront(1.0) * moveSpeed));
        };

        auto moveUp = [&player, &currentCamera, &moveSpeed]() {
            player.updatePos(player.getPos(1.0) + glm::vec3{ 0, moveSpeed, 0 });
        };

        auto moveDown = [&player, &currentCamera, &moveSpeed]() {
            player.updatePos(player.getPos(1.0) - glm::vec3{ 0, moveSpeed, 0 });
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
