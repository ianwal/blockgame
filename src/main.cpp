#include "src/actions.h"
#include "src/app_context.h"
#include "src/block.h"
#include "src/button.h"
#include "src/camera.h"
#include "src/game.h"
#include "src/input_manager.h"
#include "src/renderer.h"
#include "src/shaders.h"
#include "src/time.h"
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

using alf::AppContext;
using alf::Timestamp;

namespace alf
{

namespace
{

// Must only be called in the main thread.
bool isWindowFullscreen(SDL_Window* window)
{
    return (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN);
}

// Must only be called in the main thread.
void setWindowFullscreen(SDL_Window* window, bool fullscreen)
{
    SDL_SetWindowFullscreen(window, fullscreen);
    SDL_Log("Changed window to %s", fullscreen ? "windowed" : "fullscreen");
    SDL_SyncWindow(window);
}

// Must only be called in the main thread.
void toggleWindowFullscreen(SDL_Window* window)
{
    setWindowFullscreen(window, !isWindowFullscreen(window));
}

} // namespace

std::chrono::nanoseconds calculate_frame_time(Timestamp previous_frame, Timestamp current_frame)
{
    return current_frame - previous_frame;
}

extern "C" SDL_AppResult SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

extern "C" SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO)) {
        // Success.
    } else {
        return SDL_Fail();
    }

    if (TTF_Init()) {
        // Success.
    } else {
        return SDL_Fail();
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Request OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

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

    alf::global_font_init();

    // Load assets

    alf::ShaderManagerSingleton::get()
        .load("vertex_shader", "assets/shaders/vertex_shader.glsl", "assets/shaders/vertex_shader.frag")
        .load("cube", "assets/shaders/cube.glsl", "assets/shaders/cube.frag")
        .load("crosshair", "assets/shaders/crosshair.glsl", "assets/shaders/crosshair.frag");

    alf::TextureResourceManagerSingleton::get()
        .load("block.cobble", "assets/textures/cobble.png")
        .load("block.lucky", "assets/textures/lucky.png");

    alf::Renderer::init();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();

    // Make the cursor invisible and capture it.
    SDL_HideCursor();
    SDL_SetWindowRelativeMouseMode(window, true);

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
        window,
    };

    SDL_Log("Application started successfully!");

    return SDL_APP_CONTINUE;
}

extern "C" SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    auto& app = *static_cast<AppContext*>(appstate);
    auto& ev  = *event;

    ImGui_ImplSDL3_ProcessEvent(event);

    switch (ev.type) {
        case SDL_EVENT_MOUSE_MOTION:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN: {
            app.game->input_event_buffer->push(ev);
        } break;
        case SDL_EVENT_QUIT: {
            app.app_quit = SDL_APP_SUCCESS;
        } break;
        case SDL_EVENT_WINDOW_RESIZED: {
            int w{};
            int h{};
            SDL_GetWindowSize(SDL_GetWindowFromID(event->window.windowID), &w, &h);
            SDL_Log("Window size: %dx%d", w, h);
            // Update the GL viewport after resizing, otherwise only the original window region will be rendered to.
            glViewport(0, 0, w, h);
        } break;
        default: {
            // SDL_Log("DEFAULT EVENT\n");
        } break;
    }

    return SDL_APP_CONTINUE;
}

extern "C" SDL_AppResult SDL_AppIterate(void* appstate)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    auto& app                = *static_cast<AppContext*>(appstate);
    auto const previous_time = app.current_time;
    app.current_time         = alf::get_time();
    assert(app.current_time.count() > 0);

    try {
        static bool added_button = [&app]() {
            added_button             = true;
            auto const test_callback = []() { SDL_Log("Button pressed!"); };
            alf::Button button{ test_callback };
            SDL_FRect rect{ 100, 100, 100, 100 };
            alf::VisibleButton visible_button{ button, rect };
            app.game->gui_objects.add(std::move(visible_button));
            alf::TextButton text_button{ alf::Text{ "test" }, alf::VisibleButton{ alf::Button{ test_callback },
                                                                                  SDL_FRect{ 200, 200, 50, 50 } } };
            text_button.set_pos(SDL_Point{ 500, 500 });
            app.game->gui_objects.add(std::move(text_button));
            return true;
        }();

        std::vector<InputAction> pendingActions{};

        static bool gameInputManagerInitialized = false;
        if (!gameInputManagerInitialized) {
            app.game->input_manager.registerCallbackOnRelease(SDLK_F3, [&app]() {
                if (app.game->debug_window_on) {
                    app.game->debug_window_on = false;
                    SDL_HideCursor();
                    SDL_SetWindowRelativeMouseMode(app.window, true);
                } else {
                    app.game->debug_window_on = true;
                    SDL_ShowCursor();
                    SDL_SetWindowRelativeMouseMode(app.window, false);
                }
            });

            app.game->input_manager.registerCallbackOnRelease(SDLK_F11,
                                                              [&app]() { toggleWindowFullscreen(app.window); });

            app.game->input_manager.registerCallbackOnDown(SDLK_LSHIFT, [&app]() { app.game->fastNoClipSpeed = true; });

            app.game->input_manager.registerCallbackOnRelease(
                SDL_BUTTON_LEFT, [&app, &pendingActions]() { pendingActions.push_back(SwingAction{}); });

            app.game->input_manager.registerCallbackOnRelease(SDLK_LSHIFT,
                                                              [&app]() { app.game->fastNoClipSpeed = false; });

            app.game->input_manager.registerCallbackOnRelease(SDLK_ESCAPE, [&app]() {
                // Quit the game.
                std::quick_exit(0); // TODO: Just for debugging for instant exit.
                SDL_Event quit_event{};
                quit_event.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&quit_event);
            });

            gameInputManagerInitialized = true;
        }

        // Assume nothing is changed, overwrite later.
        alf::Inputs curr_inputs = app.game->prev_inputs;

        // TODO: The first time prev_inputs is set, it should be set from the player inputs, otherwise if you hold
        // a key when the game is starting it won't be registered.
        bool mouse_updated = false;
        while (auto input_opt = app.game->input_event_buffer->consume()) {
            auto& input            = input_opt.value();
            static int input_count = 0;
            auto& text_button      = std::get<alf::TextButton>(app.game->gui_objects.objects[1]);
            text_button.update_text_str(std::to_string(input_count));
            ++input_count;
            // SDL_Log("debug: Received event.\n Event type: %d", input.type);

            if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_ESCAPE)) {
                if (!app.game->prev_inputs.esc) {
                    app.game->input_manager.queueInputOnDown(SDLK_ESCAPE);
                }
                app.game->input_manager.queueInputOnHold(SDLK_ESCAPE);
                curr_inputs.esc = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_ESCAPE)) {
                app.game->input_manager.queueInputOnRelease(SDLK_ESCAPE);
                curr_inputs.esc = false;
            } else if ((input.type == SDL_EVENT_MOUSE_BUTTON_DOWN) && (input.button.button == SDL_BUTTON_LEFT)) {
                if (!app.game->prev_inputs.mouse1) {
                    // TODO: This is not a mouse button, so converting to a SDL_Keycode is wrong. But it doesn't collide
                    // any keycodes, so it's fine for now.
                    app.game->input_manager.queueInputOnDown(SDL_BUTTON_LEFT);
                }
                // TODO: This doesn't work. Button/mouse event is different than keyboard events.
                app.game->input_manager.queueInputOnHold(SDL_BUTTON_LEFT);
                curr_inputs.mouse1 = true;
            } else if ((input.type == SDL_EVENT_MOUSE_BUTTON_UP) && (input.button.button == SDL_BUTTON_LEFT)) {
                app.game->input_manager.queueInputOnRelease(SDL_BUTTON_LEFT);
                curr_inputs.mouse1 = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_A)) {
                if (!app.game->prev_inputs.a) {
                    app.game->input_manager.queueInputOnDown(SDLK_A);
                }
                app.game->input_manager.queueInputOnHold(SDLK_A);
                curr_inputs.a = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_A)) {
                app.game->input_manager.queueInputOnRelease(SDLK_A);
                curr_inputs.a = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_D)) {
                if (!app.game->prev_inputs.d) {
                    app.game->input_manager.queueInputOnDown(SDLK_D);
                }
                app.game->input_manager.queueInputOnHold(SDLK_D);
                curr_inputs.d = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_D)) {
                app.game->input_manager.queueInputOnRelease(SDLK_D);
                curr_inputs.d = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_W)) {
                if (!app.game->prev_inputs.w) {
                    app.game->input_manager.queueInputOnDown(SDLK_W);
                }
                app.game->input_manager.queueInputOnHold(SDLK_W);
                curr_inputs.w = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_W)) {
                app.game->input_manager.queueInputOnRelease(SDLK_W);
                curr_inputs.w = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_S)) {
                if (!app.game->prev_inputs.s) {
                    app.game->input_manager.queueInputOnDown(SDLK_S);
                }
                app.game->input_manager.queueInputOnHold(SDLK_S);
                curr_inputs.s = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_S)) {
                app.game->input_manager.queueInputOnRelease(SDLK_S);
                curr_inputs.s = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_LCTRL)) {
                if (!app.game->prev_inputs.lctrl) {
                    app.game->input_manager.queueInputOnDown(SDLK_LCTRL);
                }
                app.game->input_manager.queueInputOnHold(SDLK_LCTRL);
                curr_inputs.lctrl = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_LCTRL)) {
                app.game->input_manager.queueInputOnRelease(SDLK_LCTRL);
                curr_inputs.lctrl = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_SPACE)) {
                if (!app.game->prev_inputs.space) {
                    app.game->input_manager.queueInputOnDown(SDLK_SPACE);
                }
                app.game->input_manager.queueInputOnHold(SDLK_SPACE);
                curr_inputs.space = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_SPACE)) {
                app.game->input_manager.queueInputOnRelease(SDLK_SPACE);
                curr_inputs.space = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_LSHIFT)) {
                if (!app.game->prev_inputs.lshift) {
                    app.game->input_manager.queueInputOnDown(SDLK_LSHIFT);
                }
                app.game->input_manager.queueInputOnHold(SDLK_LSHIFT);
                curr_inputs.lshift = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_LSHIFT)) {
                app.game->input_manager.queueInputOnRelease(SDLK_LSHIFT);
                curr_inputs.lshift = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_Z)) {
                if (!app.game->prev_inputs.z) {
                    app.game->input_manager.queueInputOnDown(SDLK_Z);
                }
                app.game->input_manager.queueInputOnHold(SDLK_Z);
                curr_inputs.z = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_Z)) {
                app.game->input_manager.queueInputOnRelease(SDLK_Z);
                curr_inputs.z = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_F3)) {
                if (!app.game->prev_inputs.f3) {
                    app.game->input_manager.queueInputOnDown(SDLK_F3);
                }
                app.game->input_manager.queueInputOnHold(SDLK_F3);
                curr_inputs.f3 = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_F3)) {
                app.game->input_manager.queueInputOnRelease(SDLK_F3);
                curr_inputs.f3 = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_F11)) {
                if (!app.game->prev_inputs.f11) {
                    app.game->input_manager.queueInputOnDown(SDLK_F11);
                }
                app.game->input_manager.queueInputOnHold(SDLK_F11);
                curr_inputs.f11 = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_F11)) {
                app.game->input_manager.queueInputOnRelease(SDLK_F11);
                curr_inputs.f11 = false;
            } else if ((input.type == SDL_EVENT_MOUSE_MOTION)) {
                mouse_updated    = true;
                curr_inputs.x    = input.motion.x;
                curr_inputs.y    = input.motion.y;
                curr_inputs.xRel = input.motion.xrel;
                curr_inputs.yRel = input.motion.yrel;
            } else {
            }
        }

        // Process input queues.
        app.game->input_manager.processQueues();

        // Process game state.

        // Movement speed and camera sensitivity.
        float const moveSpeed = 0.1 * app.game->moveSpeed * (app.game->fastNoClipSpeed ? 2.0f : 1.0f);
        constexpr float mouseSensitivity{ 0.5 };

        auto calculateNewYaw = [](float xRel, float yaw, float mouseSensitivity) {
            return yaw + (xRel * mouseSensitivity);
        };

        auto calculateNewPitch = [](float yRel, float pitch, float mouseSensitivity) {
            return std::clamp(pitch - (yRel * mouseSensitivity), -89.0f, 89.0f);
        };

        // Update camera
        if (mouse_updated) {
            app.game->yaw   = calculateNewYaw(curr_inputs.xRel, app.game->yaw, mouseSensitivity);
            app.game->pitch = calculateNewPitch(curr_inputs.yRel, app.game->pitch, mouseSensitivity);
        }

        app.game->cameraFront = alf::cameraFront(app.game->yaw, app.game->pitch);

        auto moveLeft = [&app, &moveSpeed]() {
            app.game->cameraPos -= glm::normalize(glm::cross(app.game->cameraFront, app.game->cameraUp)) * moveSpeed;
        };

        auto moveRight = [&app, &moveSpeed]() {
            app.game->cameraPos += glm::normalize(glm::cross(app.game->cameraFront, app.game->cameraUp)) * moveSpeed;
        };

        auto moveForward = [&app, &moveSpeed]() { app.game->cameraPos += app.game->cameraFront * moveSpeed; };

        auto moveBackward = [&app, &moveSpeed]() { app.game->cameraPos -= app.game->cameraFront * moveSpeed; };

        auto moveUp = [&app, &moveSpeed]() { app.game->cameraPos.y += moveSpeed; };

        auto moveDown = [&app, &moveSpeed]() { app.game->cameraPos.y -= moveSpeed; };

        // Update positions
        if (curr_inputs.a == true) {
            moveLeft();
        }
        if (curr_inputs.d == true) {
            moveRight();
        }
        if (curr_inputs.w == true) {
            moveForward();
        }
        if (curr_inputs.s == true) {
            moveBackward();
        }
        if (curr_inputs.lctrl == true) {
            moveDown();
        }
        if (curr_inputs.space == true) {
            moveUp();
        }

        if (app.game->debug_window_on) {
            ImGui::ShowDemoWindow();
        }

        // Game logic

        for (auto&& action : pendingActions) {
            std::visit(alf::overloaded{
                           [&](alf::SwingAction&) { handleSwingAction(*app.game); },
                           [&](alf::MoveAction& a) { /*handleMove(a.direction);*/ },
                       },
                       action);
        }

        // Rendering

        alf::Renderer::render(app);

        // Limit fps to 300
        auto const fps = alf::calculate_fps(previous_time, alf::get_time());
        // std::cout << std::to_string(fps) << '\n';
        // if (fps > 300) {
        // SDL_Delay(1);
        // }

        app.game->prev_inputs = curr_inputs;

        pendingActions.clear();

        ++app.game->frame_num;
    } catch (const std::exception& e) {
        SDL_Log("%s", e.what());
        app.app_quit = SDL_APP_FAILURE;
    }

    return app.app_quit;
}

extern "C" void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    auto* app = static_cast<AppContext*>(appstate);
    delete app;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    alf::destroy_global_font();

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}

} // namespace alf
