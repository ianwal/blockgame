#include "src/renderer.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "src/app_context.h"
#include "src/block.h"
#include "src/block_renderer.h"
#include "src/button.h"
#include "src/chunkrenderer.h"
#include "src/crosshair_renderer.h"
#include "src/game.h"
#include "src/logger.h"
#include "src/shaders.h"
#include "src/time.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#ifdef BLOCKGAME_ENABLE_TRACY
#include <tracy/Tracy.hpp>
#endif

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

namespace bg
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

GLuint Renderer::viewProjectionUboID;

void Renderer::init()
{
    glGenBuffers(1, &viewProjectionUboID);
    glBindBuffer(GL_UNIFORM_BUFFER, viewProjectionUboID);
    // Upload 2 4x4 matrices - one is the view matrix the other is the projection matrix.
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, viewProjectionUboID); // bind to slot 0

    // BlockRenderer::init();
    ChunkRenderer::init();
    CrosshairRenderer::init();
}

void Renderer::cleanup()
{
    glDeleteBuffers(1, &viewProjectionUboID);
    // BlockRenderer::cleanup();
    ChunkRenderer::cleanup();
    CrosshairRenderer::cleanup();
}

void Renderer::render(AppContext& ctx, float partialTick)
{
    // Clear context, draw background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.25, 0.6, 1, 1);

    auto windowSize                 = getWindowSize(ctx);
    float const fov                 = 90.0f;
    float const near_clipping_plane = 0.1f;   // Keep as big as possible or you'll get precision issues.
    float const far_clipping_plane  = 100.0f; // Keep as small as possible.
    glm::vec3 cameraPos = ctx.game->player.currentCamera->getPos(partialTick); // Camera position in world space.
    glm::vec3 cameraFront =
        ctx.game->player.currentCamera->getFront(partialTick); // Front of the camera in world space.
    glm::vec3 cameraUp = ctx.game->player.currentCamera->getUp();

    glm::mat4 projection = glm::perspective(
        glm::radians(fov), static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height),
        near_clipping_plane, far_clipping_plane);
    glm::mat4 view = glm::lookAt(cameraPos,               // Camera is at this coordinate in world space
                                 cameraPos + cameraFront, // Camera looks at this coordinate.
                                 cameraUp                 // Camera head is up (0, -1, 0) would be looking upside-down)
    );

    // Upload projection and view matrices to be used by the other renderers.
    glBindBuffer(GL_UNIFORM_BUFFER, viewProjectionUboID);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    {
#ifdef BLOCKGAME_ENABLE_TRACY
        ZoneScopedN("ChunkRenderer");
#endif
        ChunkRenderer::render(ctx, ctx.game->chunkManager, partialTick);
    }

    if (ctx.game->ingameHudVisible) {
        CrosshairRenderer::render(ctx);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap the OpenGL buffer to draw to the window.
    auto swapWindowSuccess = SDL_GL_SwapWindow(ctx.window);
    if (swapWindowSuccess) {
        // Success.
    } else {
        logError("SDL_GL_SwapWindow() failure: {}", SDL_GetError());
        assert(false);
    }
}

} // namespace bg
