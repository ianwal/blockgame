#include "src/gui/gui_esc_menu.h"

#include "imgui.h"
#include "src/app_context.h"
#include "src/game.h"

#include <SDL3/SDL.h>

#include <format>
#include <string>

namespace bg
{

void GuiEscMenu::open()
{
    if (!isopen) {
        isopen = true;
        SDL_ShowCursor();
        SDL_SetWindowRelativeMouseMode(App::get().window, false);
        auto const windowSize = getWindowSize(App::get());
        SDL_WarpMouseInWindow(App::get().window, windowSize.width / 2.0f, windowSize.height / 2.0f);
        pauseGame(*App::get().game);
    }
}

void GuiEscMenu::close()
{
    if (isopen) {
        isopen = false;
        SDL_HideCursor();
        SDL_SetWindowRelativeMouseMode(App::get().window, true);
        unpauseGame(*App::get().game);
    }
}

bool GuiEscMenu::isOpen() const
{
    return isopen;
}

void GuiEscMenu::render()
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
    ImGui::SetNextWindowPos(ImVec2{ gameWindowSize.x / 2.0f, gameWindowSize.y / 2.0f }, ImGuiCond_Always);
    ImGui::Begin("ESC Menu", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    if (ImGui::Button("Resume Game", ImVec2(120.0f, 0.0f))) {
        this->close();
    }

    if (ImGui::Button("Quit Game", ImVec2(120.0f, 0.0f))) {
        quitGame(*App::get().game);
    }

    ImGui::Text("Options");
    ImGui::SliderFloat("FOV", &App::get().gameOptions.fov, 70.0f, 110.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderInt("Simulation Distance", reinterpret_cast<int*>(&App::get().gameOptions.chunkLoadRadius), 1, 32,
                     "%d", ImGuiSliderFlags_AlwaysClamp);
    ImGui::SliderFloat("Mouse Sensitivity", &App::get().gameOptions.mouseSensitivity, 0.0f, 1.0f, "%.2f",
                       ImGuiSliderFlags_AlwaysClamp);

    ImGui::End();
}

} // namespace bg
