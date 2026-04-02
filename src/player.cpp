#include "src/player.h"

#include <glm/glm.hpp>

glm::vec3 Player::getPos(float partialTick)
{
    return lerp(pos, prevPos, partialTick);
}
