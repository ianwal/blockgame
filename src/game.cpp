#include "src/game.h"

#include "src/block.h"
#include "src/camera.h"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <limits>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace alf
{

GameContext::GameContext()
{
    // Generate some blocks.
    SDL_Log("debug: Generating chunks...");
    constexpr int chunk_size{ 8 };
    constexpr int num_chunks{ 4 };
    for (int chunk_index = 0; chunk_index < num_chunks; ++chunk_index) {
        for (int i = 0; i < chunk_size; ++i) {
            for (int j = 0; j < chunk_size; ++j) {
                for (int k = 0; k < chunk_size; ++k) {
                    Block block{};
                    bool const isTopLayer = j == (chunk_size - 1);
                    if (isTopLayer) {
                        block.type = BlockType::BLOCK_GRASS;
                    } else {
                        block.type = BlockType::BLOCK_COBBLE;
                        // block.type = static_cast<BlockType>(SDL_rand(2));
                    }
                    block.pos = glm::vec3{ (chunk_index * chunk_size) + i, j, k };
                    blocks.push_back(block);
                }
            }
        }
    }
    SDL_Log("debug: DONE generating chunks.");
}

void handleSwingAction(GameContext& game)
{
    glm::vec3 normalizedCameraFront = glm::normalize(game.cameraFront);
    constexpr float maxHitRange     = 5.f;
    constexpr float step            = 0.05f;

    auto hitBlock  = game.blocks.end();
    float closestT = std::numeric_limits<float>::max();

    for (auto block_it = game.blocks.begin(); block_it != game.blocks.end(); ++block_it) {
        auto const& block = *block_it;

        // Optimization: This skips checking the logic if the block is beyond the max hitrange.
        if (glm::distance(game.cameraPos, block.pos) > maxHitRange + 1.f) {
            continue;
        }

        for (float t = 0; t < maxHitRange; t += 0.1f) {
            glm::vec3 ray = game.cameraPos + (normalizedCameraFront * t);
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

    // SDL_Log("Camera Pos: %s Angle: %s", glm::to_string(app.game->cameraPos).c_str(),
    //         glm::to_string(app.game->cameraFront).c_str());
}

} // namespace alf
