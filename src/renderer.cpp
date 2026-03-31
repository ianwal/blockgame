#include "src/renderer.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "src/app_context.h"
#include "src/block.h"
#include "src/block_renderer.h"
#include "src/button.h"
#include "src/crosshair_renderer.h"
#include "src/game.h"
#include "src/shaders.h"
#include "src/time.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace alf
{

std::int64_t calculate_fps(Timestamp previous_frame, Timestamp current_frame)
{
    auto const frame_time{ current_frame - previous_frame };
    constexpr auto frames = decltype(frame_time){ std::chrono::seconds{ 1 } };
    // auto const frame_time_ms = frame_time.count() > 0 ? frame_time.count() : 1000;

    assert(frame_time.count() > 0);

    if (frame_time.count() == 0) {
        return std::int64_t{ 999 };
    }

    auto const fps = (static_cast<float>(frames.count()) / static_cast<float>(frame_time.count()));
    return static_cast<std::int64_t>(fps);
}

void Renderer::init()
{
    BlockRenderer::init();
    CrosshairRenderer::init();
}

void Renderer::cleanup()
{
}

void Renderer::render(AppContext& ctx)
{
    // Clear context, draw background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.25, 0.6, 1, 1);

    for (auto const& block : ctx.game->blocks) {
        BlockRenderer::render(ctx, block);
    }

    CrosshairRenderer::render(ctx);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap the OpenGL buffer to draw to the window.
    auto swapWindowSuccess = SDL_GL_SwapWindow(ctx.window);
    if (swapWindowSuccess) {
        // Success.
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_GL_SwapWindow() failure: %s", SDL_GetError());
        assert(false);
    }
}

} // namespace alf
