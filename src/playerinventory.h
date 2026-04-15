#pragma once

#include "src/item.h"

#include <array>
#include <cstdint>
#include <optional>

namespace bg
{

struct InventorySlot {
    std::uint32_t x{};
    std::uint32_t y{};
};

struct ItemStack {
    ItemType item{};
    std::uint32_t amount{};
};

class PlayerInventory
{
private:
    static constexpr std::uint32_t numColumns = 9;
    static constexpr std::uint32_t numRows    = 5;

public:
    PlayerInventory();

    /// Try to add an item to the inventory in the first available space.
    ///
    /// @return The amount of items added. If the full amount can't be added then a partial amount will be added.
    std::uint32_t addItem(ItemStack itemStack);

    // std::uint32_t moveItem(InventorySlot oldSlot, InventorySlot newSlot, std::uint32_t amount);

    /// Try to add an item to the inventory in the first available space.
    ///
    /// @return If all the items were successfully removed. Partial amounts will not be removed on failure.
    bool removeItem(InventorySlot slot, std::uint32_t amount);

    void setCurrentHotbarSlot(std::uint32_t slotX);

    std::uint32_t getCurrentHotbarSlot() const;

    std::optional<ItemStack> getCurrentHeldItem() const;

    std::array<std::array<std::optional<ItemStack>, numColumns>, numRows> const& getInventory() const;

    static constexpr std::uint32_t getNumColumns()
    {
        return numColumns;
    }

    static constexpr std::uint32_t getNumRows()
    {
        return numRows;
    }

    static constexpr std::uint32_t getHotbarSize()
    {
        return numColumns;
    }

private:
    std::array<std::array<std::optional<ItemStack>, numColumns>, numRows> inventorySlots{};

    std::uint32_t currentHotbarSlot{ 0U };
};

} // namespace bg
