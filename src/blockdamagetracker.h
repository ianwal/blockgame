#pragma once

#include "src/block.h"

#include <cstdint>

namespace bg
{

/// Track block damage and play damage sounds.
///
/// @note This class is not very well written. Probably will need to be redone at some point.
class BlockDamageTracker
{
public:
    /// Damage the block.
    void damageBlock(Block const& block, float damage, std::uint64_t currentIngameTickCounter);

    /// Get the block damage progress from 0.0 to 1.0.
    float getDamageProgress() const;

    Block const& getCurrentBlock() const;

    void tick(std::uint64_t currentIngameTickCounter);

private:
    Block currentBlock{};
    float currentDamage{};
    std::uint64_t lastDamageTick{};
    std::uint32_t ticksSinceLastHit{ 0 };
    static constexpr std::uint32_t ticksUntilResetProgress{ 2 };

    std::uint64_t prevSoundLastTick{};
    static constexpr std::uint32_t minTicksBetweenPlaySound{ 8 };
};

} // namespace bg
