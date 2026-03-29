#include "src/renderer.h"
#include "src/app_context.h"
#include "src/block.h"
#include "src/block_renderer.h"
#include "src/button.h"
#include "src/game.h"
#include "src/shaders.h"
#include "src/time.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <string>
#include <string_view>

namespace Alf
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

void render(AppContext& app_ctx)
{
    auto& app = app_ctx;
    // draw a color
    auto const current_time = Alf::get_time();
    auto const time =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds{ current_time }).count() /
        1000.f;
    auto const red   = (std::sin(time) + 1) / 2.0 * 255;
    auto const green = (std::sin(time / 2) + 1) / 2.0 * 255;
    auto const blue  = (std::sin(time) * 2 + 1) / 2.0 * 255;

    SDL_Color const forecol{ 0, 0, 0, 255 };

    SDL_SetRenderDrawColor(app.renderer, red, green, blue, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(app.renderer);

    auto const previous_time = app.current_time;
    auto const fps           = calculate_fps(previous_time, current_time);
    app.current_time         = current_time;

    std::string text{};
    text = std::to_string(fps);

    SDL_Surface* text_surface{ TTF_RenderUTF8_Solid(&get_font(), text.c_str(), forecol) };
    SDL_Texture* text_texture{ SDL_CreateTextureFromSurface(app.renderer, text_surface) };
    auto const text_width  = text_surface->w;
    auto const text_height = text_surface->h;

    SDL_FRect const renderQuad{ 0.0f, 0.0f, (float) text_width, (float) text_height };
    SDL_RenderTexture(app.renderer, text_texture, nullptr, &renderQuad);
    SDL_DestroySurface(text_surface);
    SDL_DestroyTexture(text_texture);

    app.game->gui_objects.render(*app.renderer);

    // SDL_RenderPresent(app.renderer);

    SDL_GL_SetSwapInterval(1);
    // Clear context
    glClearColor(0.25, 0.6, 1, 1);

    // Create Vertex Array Object and set as current.
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    for (auto const& block : app_ctx.game->blocks) {
        BlockRenderer::render(app_ctx, block);
    }

    auto swapWindowSuccess = SDL_GL_SwapWindow(app.window);
    if (swapWindowSuccess != SDL_TRUE) {
        std::cerr << "SDL_GL_SwapWindow() failure: " << SDL_GetError() << '\n';
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // SDL_Log("FPS: %ld Frame time: %ld ns", fps, calculate_frame_time(previous_time, current_time).count());
    // SDL_Log("%ld", app.game->frame_num);
    // std::cout << "Camera position: " << to_str(app.game->cameraPos) << '\n';
    ++app.game->frame_num;
}

} // namespace Alf
