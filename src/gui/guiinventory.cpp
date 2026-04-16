#include "src/gui/guiinventory.h"

#include "imgui.h"
#include "src/app_context.h"
#include "src/blockitem.h"
#include "src/entitydroppeditem.h"
#include "src/logger.h"
#include "src/playerinventory.h"

#include <SDL3/SDL.h>

#include <format>
#include <string>

namespace bg
{

void GuiInventory::open()
{
    if (!isopen) {
        isopen = true;
        SDL_ShowCursor();
        SDL_SetWindowRelativeMouseMode(App::get().window, false);
        auto const windowSize = getWindowSize(App::get());
        SDL_WarpMouseInWindow(App::get().window, windowSize.width / 2.0f, windowSize.height / 2.0f);
    }
}

void GuiInventory::close()
{
    if (isopen) {
        isopen = false;
        SDL_HideCursor();
        SDL_SetWindowRelativeMouseMode(App::get().window, true);
    }
}

bool GuiInventory::isOpen() const
{
    return isopen;
}

std::map<ItemType, std::uint32_t> getAllInventoryItems(PlayerInventory::InventoryType const& inventory)
{
    std::map<ItemType, std::uint32_t> items{};

    for (auto const& inventorySlotRow : inventory) {
        for (auto const& inventorySlot : inventorySlotRow) {
            if (!inventorySlot.has_value()) {
                continue;
            }
            if (!items.contains(inventorySlot->item)) {
                items[inventorySlot->item] = 0;
            }
            items[inventorySlot->item] += inventorySlot->amount;
        }
    }

    return items;
}

void GuiInventory::render(PlayerInventory const& playerInventory)
{
    // Tint the background game black.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##tint", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, IM_COL32(0, 0, 0, 200));
    ImGui::End();

    ImVec2 const gameWindowSize = ImGui::GetIO().DisplaySize;
    auto const& inventory       = playerInventory.getInventory();
    ImVec2 const imageSize{ 56, 56 };
    float const inventoryWindowWidth  = inventory[0].size() * (imageSize.x);
    float const inventoryWindowHeight = imageSize.y;
    ImGui::SetNextWindowPos(
        ImVec2{ (gameWindowSize.x - inventoryWindowWidth) * 0.5f, gameWindowSize.y - inventoryWindowHeight - 512 },
        ImGuiCond_Always);
    ImGui::Begin("Inventory", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    for (std::int32_t i = static_cast<std::int32_t>(inventory.size()) - 1; i > -1; --i) {
        for (std::int32_t j = 0; j < static_cast<std::int32_t>(inventory[i].size()) - 1; ++j) {
            auto const& inventorySlot = inventory[i][j];
            if (inventorySlot.has_value()) {
                // Item icon
                // Note: uv is flipped because OpenGL convention is flipped UV, so otherwise it would be upside-down.
                ImGui::Image((ImTextureID) (intptr_t) getItemTexture(inventorySlot->item), imageSize, ImVec2{ 0, 1 },
                             ImVec2{ 1, 0 });

                ImVec2 const botRight         = ImGui::GetItemRectMax();
                std::string const itemSizeStr = std::format("{}", inventorySlot->amount);
                ImVec2 const textSize         = ImGui::CalcTextSize(itemSizeStr.c_str());
                ImVec2 const textPos          = ImVec2(botRight.x - textSize.x - 5, botRight.y - textSize.y - 5);
                // Item count text shadow
                ImGui::GetWindowDrawList()->AddText(ImVec2{ textPos.x + 1, textPos.y + 1 }, IM_COL32(0, 0, 0, 250),
                                                    itemSizeStr.c_str());
                // Item count text
                ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), itemSizeStr.c_str());
            } else {
                ImGui::Image((ImTextureID) 0, imageSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
            }

            // Use same row for next image
            if (j < static_cast<std::int32_t>(inventory[i].size()) - 2) {
                ImGui::SameLine(0, 10);
            }
        }
        // Vertical spacing for the hotbar row
        if (i == 1) {
            ImGui::Dummy(ImVec2{ 0, 0 });
            ImGui::Dummy(ImVec2{ 0, 0 });
            ImGui::Dummy(ImVec2{ 0, 0 });
        }
    }
    ImGui::End();

    auto const& allCraftingRecipes = App::get().craftingRecipeRegistry.getAllRecipes();
    auto const allInventoryItems   = getAllInventoryItems(playerInventory.getInventory());

    auto canCraftAny = [&allInventoryItems, &allCraftingRecipes]() {
        for (auto const& recipe : allCraftingRecipes) {
            bool canCraft = true;
            for (auto const& recipeInput : recipe.inputs) {
                if (!allInventoryItems.contains(recipeInput.item)) {
                    canCraft = false;
                    break;
                }
                if (allInventoryItems.at(recipeInput.item) < recipeInput.amount) {
                    canCraft = false;
                    break;
                }
            }
            if (canCraft) {
                return true;
            }
        }
        return false;
    };

    if (canCraftAny()) {
        ImGui::Begin("Crafting", nullptr,
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
        std::uint32_t recipeIndex = 0;
        for (auto const& recipe : allCraftingRecipes) {
            // Item icon
            // Note: uv is flipped because OpenGL convention is flipped UV, so otherwise it would be upside-down.
            bool canCraft = true;
            for (auto const& recipeInput : recipe.inputs) {
                if (!allInventoryItems.contains(recipeInput.item)) {
                    canCraft = false;
                    break;
                }
                if (allInventoryItems.at(recipeInput.item) < recipeInput.amount) {
                    canCraft = false;
                    break;
                }
            }

            if (!canCraft) {
                continue;
            }

            if (ImGui::ImageButton(std::format("{}", recipeIndex).c_str(),
                                   (ImTextureID) (intptr_t) getItemTexture(recipe.output.item), imageSize,
                                   ImVec2{ 0, 1 }, ImVec2{ 1, 0 })) {
                CraftingRecipe recipeCopy = recipe;
                for (std::uint32_t i = 0; i < recipe.output.amount; ++i) {
                    auto droppedItem = std::make_unique<EntityDroppedItem>();
                    droppedItem->setItemType(recipe.output.item);
                    droppedItem->updatePos(App::get().game->player.fpCamera.getPos(1.0f) +
                                           (App::get().game->player.fpCamera.getFront(1.0f) * 0.3f));
                    droppedItem->setVelocity(App::get().game->player.fpCamera.getFront(1.0f) * 0.2f);
                    App::get().game->levelManager.addEntity(std::move(droppedItem));
                }

                for (auto recipeInput : recipe.inputs) {
                    bool done = false;
                    for (std::uint32_t i = 0; i < PlayerInventory::getNumRows(); ++i) {
                        for (std::uint32_t j = 0; j < PlayerInventory::getNumColumns(); ++j) {
                            if (inventory[i][j].has_value() && (inventory[i][j]->item == recipeInput.item)) {
                                recipeInput.amount -= App::get().game->playerInventory.removeItemPartial(
                                    InventorySlot{ i, j }, recipeInput.amount);

                                if (recipeInput.amount == 0) {
                                    done = true;
                                    break;
                                }
                            }
                        }
                        if (done) {
                            break;
                        }
                    }
                    assertr(recipeInput.amount == 0, std::format("{}", recipeInput.amount));
                }

                ImVec2 const botRight         = ImGui::GetItemRectMax();
                std::string const itemSizeStr = std::format("{}", recipe.output.amount);
                ImVec2 const textSize         = ImGui::CalcTextSize(itemSizeStr.c_str());
                ImVec2 const textPos          = ImVec2(botRight.x - textSize.x - 5, botRight.y - textSize.y - 5);
                // Item count text shadow
                ImGui::GetWindowDrawList()->AddText(ImVec2{ textPos.x + 1, textPos.y + 1 }, IM_COL32(0, 0, 0, 250),
                                                    itemSizeStr.c_str());
                // Item count text
                ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), itemSizeStr.c_str());

                // Use same row for next image
                std::int32_t const itemsPerRow{ 2 };
                if ((recipeIndex % itemsPerRow) == 0) {
                    ImGui::SameLine(0, 10);
                }
            }
            ++recipeIndex;
        }
        ImGui::End();
    }
}

} // namespace bg
