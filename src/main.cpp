#include "src/button.h"
#include "src/game.h"
#include "src/shaders.h"
#include "src/time.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

using Alf::Timestamp;

struct AppContext {
    AppContext(SDL_Renderer* renderer, SDL_Window* window) : renderer{ renderer }, window{ window }
    {
        assert(this->renderer != nullptr);
        assert(this->window != nullptr);
    }

    SDL_Renderer* renderer{};
    SDL_Window* window{};
    std::unique_ptr<Alf::GameContext> game{ std::make_unique<Alf::GameContext>() };

    SDL_AppResult app_quit{ SDL_APP_CONTINUE };
    Timestamp current_time{};

    ~AppContext()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }
};

std::chrono::nanoseconds calculate_frame_time(Timestamp previous_frame, Timestamp current_frame)
{
    return current_frame - previous_frame;
}

int64_t calculate_fps(Timestamp previous_frame, Timestamp current_frame)
{
    auto const frame_time{ current_frame - previous_frame };
    constexpr auto frames = decltype(frame_time){ std::chrono::seconds{ 1 } };
    // auto const frame_time_ms = frame_time.count() > 0 ? frame_time.count() : 1000;

    assert(frame_time.count() > 0);

    if (frame_time.count() == 0) {
        return int64_t{ 999 };
    }

    auto const fps = (static_cast<float>(frames.count()) / static_cast<float>(frame_time.count()));
    return static_cast<int64_t>(fps);
}

SDL_AppResult SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO) != SDL_TRUE) {
        return SDL_Fail();
    }

    if (TTF_Init() != SDL_TRUE) {
        return SDL_Fail();
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Request OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // create a window
    SDL_Window* window = SDL_CreateWindow("Window", 1280, 720, SDL_WINDOW_OPENGL);
    if (!window) {
        return SDL_Fail();
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        return SDL_Fail();
    }

    {
        // Initialize GLEW (MUST be after OpenGL context creation)
        glewExperimental = GL_TRUE; // Fixes issues with some drivers
        GLenum err       = glewInit();
        if (err != GLEW_OK) {
            std::cerr << "GLEW Init Error: " << glewGetErrorString(err) << std::endl;
            return SDL_Fail();
        }

        std::cout << "GLEW initialized successfully!" << std::endl;

        const GLubyte* renderer_name = glGetString(GL_RENDERER);
        const GLubyte* version_name  = glGetString(GL_VERSION);
        std::cout << "Renderer: " << renderer_name << std::endl;
        std::cout << "OpenGL Version: " << version_name << std::endl;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        return SDL_Fail();
    }

    // print some information about the window
    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", width, height);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth) {
            SDL_Log("This is a highdpi environment.");
        }
    }

    // set up the application data
    *appstate = new AppContext{
        renderer,
        window,
    };

    SDL_SetRenderVSync(renderer, SDL_WINDOW_SURFACE_VSYNC_DISABLED);
    SDL_Log("Application started successfully!");

    Alf::global_font_init();

    alf::ShaderManagerSingleton::get().load("vertex_shader", "assets/shaders/vertex_shader.glsl", "assets/shaders/vertex_shader.frag");

    alf::TextureResourceManagerSingleton::get().load("logo", "assets/textures/logo.png");

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, const SDL_Event* event)
{
    auto& app = *static_cast<AppContext*>(appstate);
    auto& ev  = *event;

    switch (ev.type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN: {
            // SDL_Log("KEY DOWN OR UP\n");
            app.game->input_event_buffer->push(ev);
        } break;
        case SDL_EVENT_QUIT: {
            app.app_quit = SDL_APP_SUCCESS;
        } break;
        default: {
            // SDL_Log("DEFAULT EVENT\n");
        } break;
    }

    return SDL_APP_CONTINUE;
}

namespace Alf
{

void render(AppContext& app_ctx)
{
    auto& app = app_ctx;
    // draw a color
    auto const current_time = Alf::get_time();
    auto const time  = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds{ current_time }).count() / 1000.f;
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
    glClearColor(0, 0, 0.4, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // Create Vertex Array Object and set as current.
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };

    // This will identify our vertex buffer
    GLuint vertexbuffer;
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
                          3,        // size
                          GL_FLOAT, // type
                          GL_FALSE, // normalized?
                          0,        // stride
                          (void*) 0 // array buffer offset
    );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(0);

    glUseProgram(alf::ShaderManagerSingleton::get().get("vertex_shader"));

    auto swapWindowSuccess = SDL_GL_SwapWindow(app.window);
    if (swapWindowSuccess != SDL_TRUE) {
        std::cerr << "SDL_GL_SwapWindow() failure: " << SDL_GetError() << '\n';
    }

    // SDL_Log("FPS: %ld Frame time: %ld ns", fps, calculate_frame_time(previous_time, current_time).count());
    // SDL_Log("%ld", app.game->frame_num);
    ++app.game->frame_num;
}

} // namespace Alf

SDL_AppResult SDL_AppIterate(void* appstate)
{
    auto& app                = *static_cast<AppContext*>(appstate);
    auto const previous_time = app.current_time;
    app.current_time         = Alf::get_time();
    assert(app.current_time.count() > 0);

    try {
        static bool added_button = [&app]() {
            added_button             = true;
            auto const test_callback = []() { SDL_Log("Button pressed!"); };
            Alf::Button button{ test_callback };
            SDL_FRect rect{ 100, 100, 100, 100 };
            Alf::VisibleButton visible_button{ button, rect };
            app.game->gui_objects.add(std::move(visible_button));
            Alf::TextButton text_button{ Alf::Text{ "test" },
                                         Alf::VisibleButton{ Alf::Button{ test_callback }, SDL_FRect{ 200, 200, 50, 50 } } };
            text_button.set_pos(SDL_Point{ 500, 500 });
            app.game->gui_objects.add(std::move(text_button));
            return true;
        }();

        auto const input_opt = app.game->input_event_buffer->consume();
        if (input_opt.has_value()) {
            auto& input            = input_opt.value();
            static int input_count = 0;
            auto& text_button      = std::get<Alf::TextButton>(app.game->gui_objects.objects[1]);
            text_button.update_text_str(std::to_string(input_count));
            ++input_count;
            SDL_Log("Received event.\n Event type: %d", input.type);

            // Quit the game.
            if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_ESCAPE)) {
                SDL_Event quit_event{};
                quit_event.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&quit_event);
            }
        }

        Alf::render(app);
        // Limit fps to 300
        auto const fps = calculate_fps(previous_time, Alf::get_time());
        // std::cout << std::to_string(fps) << '\n';
        if (fps > 300) {
            SDL_Delay(1);
        }
    } catch (const std::exception& e) {
        SDL_Log("%s", e.what());
        app.app_quit = SDL_APP_FAILURE;
    }

    return app.app_quit;
}

void SDL_AppQuit(void* appstate)
{
    auto* app = static_cast<AppContext*>(appstate);
    delete app;

    Alf::destroy_global_font();

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
