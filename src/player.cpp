#include "src/player.h"

#include "src/utils.h"

#include <glm/glm.hpp>

namespace bg
{

void Player::tick()
{
    if (noclip) {

    } else {
        // TODO: Check collision
    }
}

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

float Player::getMoveSpeedMultiplier() const
{
    return moveSpeedMultiplier;
}

void Player::setMoveSpeedMultiplier(float moveSpeedMultiplier)
{
    this->moveSpeedMultiplier = moveSpeedMultiplier;
}

} // namespace bg
