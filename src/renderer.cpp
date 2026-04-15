#include "src/renderer.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "src/app_context.h"
#include "src/block.h"
#include "src/block_renderer.h"
#include "src/blockbreakprogressrenderer.h"
#include "src/button.h"
#include "src/chunkrenderer.h"
#include "src/crosshair_renderer.h"
#include "src/entitydroppeditemrenderer.h"
#include "src/game.h"
#include "src/gui_esc_menu.h"
#include "src/guihotbar.h"
#include "src/guiinventory.h"
#include "src/handrenderer.h"
#include "src/linerenderer.h"
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
#include <vector>

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

/// @brief Get the lines for rendering the hovered block outline.
std::vector<Line> getBlockOutline(Block const& block)
{
    std::vector<Line> result{};
    result.reserve(12);
    glm::vec3 const blockPos = tovec3(block.pos);
    // Note:
    // These lines are shifted a bit for all the axis so they actually appear, otherwise they would be in the crack
    // between blocks.
    float const width = 1.5f;
    glm::vec4 const color{ 0.0f, 0.0f, 0.0f, 0.0f };

    // X
    result.push_back(Line{ .start = { blockPos + glm::vec3{ -0.5f, -0.49f, -1.5f } },
                           .end   = { blockPos + glm::vec3{ -0.5f, -0.49f, -0.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ -0.5f, 0.51f, -1.5f } },
                           .end   = { blockPos + glm::vec3{ -0.5f, 0.51f, -0.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ 0.5f, 0.51f, -1.5f } },
                           .end   = { blockPos + glm::vec3{ 0.5f, 0.51f, -0.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ 0.5f, -0.49f, -1.5f } },
                           .end   = { blockPos + glm::vec3{ 0.5f, -0.49f, -0.5f } },
                           .width = width,
                           .color = color });

    // Z
    result.push_back(Line{ .start = { blockPos + glm::vec3{ -0.5f, -0.49f, -0.5f } },
                           .end   = { blockPos + glm::vec3{ 0.5f, -0.49f, -0.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ -0.5f, 0.51f, -0.5f } },
                           .end   = { blockPos + glm::vec3{ 0.5f, 0.51f, -0.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ -0.5f, 0.51f, -1.5f } },
                           .end   = { blockPos + glm::vec3{ 0.5f, 0.51f, -1.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ -0.5f, -0.49f, -1.5f } },
                           .end   = { blockPos + glm::vec3{ 0.5f, -0.49f, -1.5f } },
                           .width = width,
                           .color = color });

    // Y
    result.push_back(Line{ .start = { blockPos + glm::vec3{ -0.5f, -0.5f, -0.5f } },
                           .end   = { blockPos + glm::vec3{ -0.5f, 0.51f, -0.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ -0.5f, -0.5f, -1.5f } },
                           .end   = { blockPos + glm::vec3{ -0.5f, 0.51f, -1.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ 0.5f, -0.5f, -0.5f } },
                           .end   = { blockPos + glm::vec3{ 0.5f, 0.51f, -0.5f } },
                           .width = width,
                           .color = color });
    result.push_back(Line{ .start = { blockPos + glm::vec3{ 0.5f, -0.5f, -1.5f } },
                           .end   = { blockPos + glm::vec3{ 0.5f, 0.51f, -1.5f } },
                           .width = width,
                           .color = color });
    return result;
}

/// @brief Get the lines for rendering the chunk outline.
std::vector<Line> getChunkOutline(ChunkPos const& chunkPos)
{
    std::vector<Line> result{};
    result.reserve(12);

    // Convert chunk position to world coordinates
    float const x_min = std::floor(static_cast<float>(chunkPos.x * chunkSize));
    float const x_max = std::floor(static_cast<float>((chunkPos.x + 1) * chunkSize));
    float const z_min = std::floor(static_cast<float>(chunkPos.z * chunkSize));
    float const z_max = std::floor(static_cast<float>((chunkPos.z + 1) * chunkSize));
    float const y_min = 0.0f;
    float const y_max = static_cast<float>(chunkHeight);

    float const width = 1.5f;
    glm::vec4 const color{ 1.0f, 0.78f, 0.0f, 0.5f };

    // Bottom edges (y = 0)
    result.push_back(
        Line{ .start = { x_min, y_min, z_min }, .end = { x_max, y_min, z_min }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_max, y_min, z_min }, .end = { x_max, y_min, z_max }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_max, y_min, z_max }, .end = { x_min, y_min, z_max }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_min, y_min, z_max }, .end = { x_min, y_min, z_min }, .width = width, .color = color });

    // Top edges (y = chunkHeight)
    result.push_back(
        Line{ .start = { x_min, y_max, z_min }, .end = { x_max, y_max, z_min }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_max, y_max, z_min }, .end = { x_max, y_max, z_max }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_max, y_max, z_max }, .end = { x_min, y_max, z_max }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_min, y_max, z_max }, .end = { x_min, y_max, z_min }, .width = width, .color = color });

    // Vertical edges
    result.push_back(
        Line{ .start = { x_min, y_min, z_min }, .end = { x_min, y_max, z_min }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_max, y_min, z_min }, .end = { x_max, y_max, z_min }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_max, y_min, z_max }, .end = { x_max, y_max, z_max }, .width = width, .color = color });
    result.push_back(
        Line{ .start = { x_min, y_min, z_max }, .end = { x_min, y_max, z_max }, .width = width, .color = color });

    return result;
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
    BlockBreakProgressRenderer::init();
    ChunkRenderer::init();
    LineRenderer::init();
    CrosshairRenderer::init();
    HandRenderer::init();
}

void Renderer::cleanup()
{
    glDeleteBuffers(1, &viewProjectionUboID);
    // BlockRenderer::cleanup();
    BlockBreakProgressRenderer::cleanup();
    ChunkRenderer::cleanup();
    LineRenderer::cleanup();
    CrosshairRenderer::cleanup();
    HandRenderer::cleanup();
}

void Renderer::render(AppContext& ctx, float partialTick)
{
    auto& game = *ctx.game;
    // Clear context, draw background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.25, 0.6, 1, 1);

    auto windowSize                 = getWindowSize(ctx);
    float const fov                 = 90.0f;
    float const near_clipping_plane = 0.1f;   // Keep as big as possible or you'll get precision issues.
    float const far_clipping_plane  = 100.0f; // Keep as small as possible.
    glm::vec3 cameraPos             = game.player.currentCamera->getPos(partialTick); // Camera position in world space.
    glm::vec3 cameraFront = game.player.currentCamera->getFront(partialTick); // Front of the camera in world space.
    glm::vec3 cameraUp    = game.player.currentCamera->getUp();

    glm::mat4 projection = glm::perspective(
        glm::radians(fov), static_cast<float>(windowSize.width) / static_cast<float>(windowSize.height),
        near_clipping_plane, far_clipping_plane);
    glm::mat4 view = glm::lookAt(cameraPos,               // Camera is at this coordinate in world space
                                 cameraPos + cameraFront, // Camera looks at this coordinate.
                                 cameraUp                 // Camera head is up <0, -1, 0> would be looking upside-down)
    );

    // Upload projection and view matrices to be used by the other renderers.
    glBindBuffer(GL_UNIFORM_BUFFER, viewProjectionUboID);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

    {
#ifdef BLOCKGAME_ENABLE_TRACY
        ZoneScopedN("ChunkRenderer");
#endif
        ChunkRenderer::render(ctx, game.chunkManager, partialTick);
    }

    for (auto const& entity : game.levelManager.getEntities()) {
        game.entityRendererManager.renderEntity(*entity, partialTick);
    }

    BlockBreakProgressRenderer::render();

    // TODO: This is probably slow to compute every frame. Pre-compute per tick and get from the player instead?
    std::optional<Block> const lookedAtBlock = getBlockPlayerIsLookingAt(game.chunkManager, game.player);
    if (lookedAtBlock.has_value()) {
        for (auto const& line : getBlockOutline(lookedAtBlock.value())) {
            LineRenderer::render(line);
        }
    }

    if (ctx.game->f3_window_on) {
        for (auto const& line : getChunkOutline(getChunkPos(game.player.getPos(partialTick)))) {
            LineRenderer::render(line);
        }
    }

    if (game.ingameHudVisible) {
        HandRenderer::render(ctx, partialTick);
        if (!game.escMenu.isOpen()) {
            CrosshairRenderer::render(ctx);
        }
    }

    if (game.escMenu.isOpen()) {
        game.escMenu.render();
    } else {
        if (game.guiInventory.isOpen()) {
            GuiInventory{}.render(game.playerInventory);
        } else {
            if (game.ingameHudVisible) {
                GuiHotbar{}.render(game.playerInventory);
            }
        }
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
