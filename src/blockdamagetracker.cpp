#include "src/blockdamagetracker.h"

#include "src/app_context.h"
#include "src/assertr.h"
#include "src/audio.h"
#include "src/block.h"
#include "src/logger.h"

#include <algorithm>
#include <format>

namespace bg
{

void BlockDamageTracker::damageBlock(Block const& block, float damage, std::uint64_t currentIngameTickCounter)
{
    if (currentBlock != block) {
        currentBlock  = block;
        currentDamage = 0.0f;
    }

    if ((currentIngameTickCounter - prevSoundLastTick) >= minTicksBetweenPlaySound) {
        playSound2d(getBlockBreakSoundName(block.type));
        prevSoundLastTick = currentIngameTickCounter;
    }

    lastDamageTick = currentIngameTickCounter;
    assertr(damage > 0.0f, std::format("{}", damage));
    float const hardness = App::get().blockRegistry.getHardness(block.type);
    if (hardness <= 0.0f) {
        currentDamage = 0.0f;
    } else {
        currentDamage += damage / (hardness * 30.0f);
        currentDamage = std::min(currentDamage, 1.0f);
    }
}

float BlockDamageTracker::getDamageProgress()
{
    return currentDamage;
}

Block const& BlockDamageTracker::getCurrentBlock() const
{
    return currentBlock;
}

void BlockDamageTracker::tick(std::uint64_t currentIngameTickCounter)
{
    if (lastDamageTick == currentIngameTickCounter) {
        ticksSinceLastHit = 0;
    } else {
        ++ticksSinceLastHit;
    }

    if (ticksSinceLastHit >= ticksUntilResetProgress) {
        currentDamage = 0.0f;
    }
}

} // namespace bg
