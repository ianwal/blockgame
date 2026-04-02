#include "src/player.h"

#include "src/utils.h"

#include <glm/glm.hpp>

namespace alf
{

glm::vec3 Player::getPos(float partialTick) const
{
    return lerp(prevPos, pos, partialTick);
}

void Player::updatePos(glm::vec3 newPos)
{
    prevPos = pos;
    pos     = newPos;
    fpCamera.updatePos(newPos);
}

} // namespace alf
