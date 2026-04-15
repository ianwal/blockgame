#include "src/playerinventory.h"

#include "src/item.h"

#include <array>
#include <cstdint>
#include <format>
#include <optional>

namespace bg
{

std::uint32_t PlayerInventory::addItem(ItemStack itemStack)
{
    // Try to add to an existing stack.
    for (auto& row : inventorySlots) {
        for (auto& slot : row) {
            if (!slot.has_value()) {
                continue;
            }
            if (slot->item == itemStack.item) {
                // TODO: Add item amounts only up to the stack amount.
                slot->amount += itemStack.amount;
                return itemStack.amount;
            }
        }
    }

    // If no existing item stacks of the same type, then try to add to an empty slot.
    for (auto& row : inventorySlots) {
        for (auto& slot : row) {
            if (!slot.has_value()) {
                // TODO: Add item amounts only up to the stack amount.
                slot = itemStack;
                return itemStack.amount;
            }
        }
    }

    return 0U;
}

bool PlayerInventory::removeItem(InventorySlot slot, std::uint32_t amount)
{
    assertr(slot.x < getNumColumns()), std::format("{}", slot.x);
    assertr(slot.y < getNumRows()), std::format("{}", slot.y);

    auto& inventorySlot = inventorySlots[slot.x][slot.y];
    if (!inventorySlot.has_value()) {
        return false;
    }

    if (inventorySlot->amount < amount) {
        return false;
    }

    inventorySlot->amount -= amount;
    if (inventorySlot->amount == 0U) {
        inventorySlot = std::nullopt;
    }

    return true;
}

void PlayerInventory::setCurrentHotbarSlot(std::uint32_t slotX)
{
    assertr(slotX < getHotbarSize(), std::format("{}", slotX));
    currentHotbarSlot = slotX;
}

std::uint32_t PlayerInventory::getCurrentHotbarSlot() const
{
    return currentHotbarSlot;
}

std::optional<ItemStack> PlayerInventory::getCurrentHeldItem() const
{
    return inventorySlots[0][currentHotbarSlot];
}

std::array<std::array<std::optional<ItemStack>, PlayerInventory::numColumns>, PlayerInventory::numRows> const&
PlayerInventory::getInventory() const
{
    return inventorySlots;
}

} // namespace bg
