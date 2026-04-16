#include "src/gui/guihotbar.h"

#include "imgui.h"
#include "src/app_context.h"
#include "src/blockitem.h"
#include "src/playerinventory.h"

#include <format>
#include <string>

namespace bg
{

void GuiHotbar::render(PlayerInventory const& playerInventory)
{
    ImGui::PushFont(App::get().mcFont, 32);
    ImVec2 const gameWindowSize = ImGui::GetIO().DisplaySize;
    auto const& inventory       = playerInventory.getInventory();
    auto const& hotbarSlots     = inventory[0];
    ImVec2 const imageSize{ 48, 48 };
    float const windowWidth  = hotbarSlots.size() * (imageSize.x);
    float const windowHeight = imageSize.y;
    ImGui::SetNextWindowPos(ImVec2{ (gameWindowSize.x - windowWidth) * 0.5f, gameWindowSize.y - windowHeight - 24 },
                            ImGuiCond_Always);
    ImGui::Begin("Hotbar", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    for (std::uint32_t hotbarSlotIndex = 0; hotbarSlotIndex < hotbarSlots.size(); ++hotbarSlotIndex) {
        auto const& inventorySlot    = hotbarSlots[hotbarSlotIndex];
        bool const isCurrentHeldSlot = playerInventory.getCurrentHotbarSlot() == hotbarSlotIndex;
        auto const borderColor       = isCurrentHeldSlot ? ImVec4{ 1, 1, 1, 1 } : ImVec4{ 0, 0, 0, 1 };
        if (inventorySlot.has_value()) {
            // Item icon
            // Note: uv is flipped because OpenGL convention is flipped UV, so otherwise it would be upside-down.
            ImGui::Image((ImTextureID) (intptr_t) getBlockTexture(toBlockType(inventorySlot->item).value()), imageSize,
                         ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4{ 1, 1, 1, 1 }, borderColor);

            ImVec2 const botRight         = ImGui::GetItemRectMax();
            std::string const itemSizeStr = std::format("{}", inventorySlot->amount);
            ImVec2 const textSize         = ImGui::CalcTextSize(itemSizeStr.c_str());
            ImVec2 const textPos          = ImVec2(botRight.x - textSize.x + 4, botRight.y - textSize.y + 4);
            // Item count text shadow
            ImGui::GetWindowDrawList()->AddText(ImVec2{ textPos.x + 1, textPos.y + 1 }, IM_COL32(0, 0, 0, 250),
                                                itemSizeStr.c_str());
            // Item count text
            ImGui::GetWindowDrawList()->AddText(textPos, IM_COL32(255, 255, 255, 255), itemSizeStr.c_str());
        } else {
            ImGui::Image((ImTextureID) 0, imageSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 }, ImVec4{ 1, 1, 1, 1 }, borderColor);
        }
        ImGui::SameLine(0, 10);
    }
    ImGui::End();
    ImGui::PopFont();
}

} // namespace bg
