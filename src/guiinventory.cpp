#include "src/guiinventory.h"

#include "imgui.h"
#include "src/app_context.h"
#include "src/blockitem.h"
#include "src/playerinventory.h"

#include <format>
#include <string>

namespace bg
{

void GuiInventory::render(PlayerInventory const& playerInventory)
{
    // Tint the background game black.
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##tint", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, IM_COL32(0, 0, 0, 120));
    ImGui::End();

    ImVec2 const gameWindowSize = ImGui::GetIO().DisplaySize;
    auto const& inventory       = playerInventory.getInventory();
    ImVec2 const imageSize{ 56, 56 };
    float const windowWidth  = inventory[0].size() * (imageSize.x);
    float const windowHeight = imageSize.y;
    ImGui::SetNextWindowPos(ImVec2{ (gameWindowSize.x - windowWidth) * 0.5f, gameWindowSize.y - windowHeight - 512 },
                            ImGuiCond_Always);
    ImGui::Begin("Inventory", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
    for (std::int32_t i = static_cast<std::int32_t>(inventory.size()) - 1; i > -1; --i) {
        for (std::int32_t j = 0; j < static_cast<std::int32_t>(inventory[i].size()) - 1; ++j) {
            auto const& inventorySlot = inventory[i][j];
            if (inventorySlot.has_value()) {
                // Item icon
                // Note: uv is flipped because OpenGL convention is flipped UV, so otherwise it would be upside-down.
                ImGui::Image((ImTextureID) (intptr_t) getBlockTexture(toBlockType(inventorySlot->item).value()),
                             imageSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

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
}

} // namespace bg
