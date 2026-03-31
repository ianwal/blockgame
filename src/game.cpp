#include "src/game.h"

#include "src/block.h"
#include "src/camera.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <limits>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace alf
{

void handleSwingAction(GameContext& game)
{
    glm::vec3 normalizedCameraFront = glm::normalize(game.cameraFront);
    constexpr float maxHitRange     = 5.f;
    constexpr float step            = 0.05f;

    bool hit = false;
    [&]() {
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
    }();

    // SDL_Log("Camera Pos: %s Angle: %s", glm::to_string(app.game->cameraPos).c_str(),
    //         glm::to_string(app.game->cameraFront).c_str());
}

} // namespace alf
