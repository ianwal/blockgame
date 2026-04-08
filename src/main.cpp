#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "src/actions.h"
#include "src/app_context.h"
#include "src/assertr.h"
#include "src/block.h"
#include "src/button.h"
#include "src/camera.h"
#include "src/chunk.h"
#include "src/game.h"
#include "src/input_manager.h"
#include "src/logger.h"
#include "src/renderer.h"
#include "src/shaders.h"
#include "src/time.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// clang-format off
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
// clang-format on

#ifdef BLOCKGAME_ENABLE_TRACY
#include <tracy/Tracy.hpp>
#endif

using bg::AppContext;
using bg::Timestamp;

namespace bg
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
    logInfo("Changed window to: {}", fullscreen ? "windowed" : "fullscreen");
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
    logCritical("SDL_Fail(): {}", SDL_GetError());
    return SDL_APP_FAILURE;
}

extern "C" SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_DEBUG, "DEBUG: ");
    SDL_SetLogPriorityPrefix(SDL_LOG_PRIORITY_INFO, "INFO: ");
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
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

    std::string const windowTitle = std::format("Block Game1 | Build: {} {}", __DATE__, __TIME__);
    SDL_Window* window = SDL_CreateWindow(windowTitle.c_str(), 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        logCritical("SDL_CreateWindow() failed: {}", SDL_GetError());
        return SDL_Fail();
    }

    SDL_SetWindowTitle(window, std::string{ std::string{ "Block Game | Build: " } + std::string{ __DATE__ } + " " +
                                            std::string{ __TIME__ } }
                                   .c_str());

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        return SDL_Fail();
    }

    {
        // Initialize GLEW (MUST be after OpenGL context creation)
        glewExperimental = GL_TRUE; // Fixes issues with some drivers
        GLenum err       = glewInit();
        if (err != GLEW_OK) {
            logCritical("GLEW Init Error: {}", reinterpret_cast<char const*>(glewGetErrorString(err)));
            return SDL_Fail();
        }

        logInfo("GLEW initialized successfully");
        logInfo("OpenGL Renderer: {}", reinterpret_cast<char const*>(glGetString(GL_RENDERER)));
        logInfo("OpenGL Renderer Supported Version: {}", reinterpret_cast<char const*>(glGetString(GL_VERSION)));
    }

    // Disable vsync
    SDL_GL_SetSwapInterval(0);

    bg::global_font_init();

    // Load assets

    bg::ShaderManagerSingleton::get()
        .load("vertex_shader", "assets/shaders/vertex_shader.glsl", "assets/shaders/vertex_shader.frag")
        .load("cube", "assets/shaders/cube.glsl", "assets/shaders/cube.frag")
        .load("crosshair", "assets/shaders/crosshair.glsl", "assets/shaders/crosshair.frag");

    bg::TextureResourceManagerSingleton::get()
        .load("block.cobble", "assets/textures/cobble.png")
        .load("block.grass", "assets/textures/grass.png")
        .load("block.dirt", "assets/textures/dirt.png")
        .load("block.lucky", "assets/textures/lucky.png");

    bg::Renderer::init();

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
        logInfo("Window size: {}x{}", width, height);
        logInfo("Backbuffer size: {}x{}", bbwidth, bbheight);
        if (width != bbwidth) {
            logInfo("highdpi environment detected.");
        }
    }

    // set up the application data
    *appstate = new AppContext{
        window,
    };

    GlobalGame::set(static_cast<AppContext*>(*appstate)->game.get());

    logInfo("Application started successfully!");

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
            logInfo("Window size: {}x{}", w, h);
            // Update the GL viewport after resizing, otherwise only the original window region will be rendered to.
            glViewport(0, 0, w, h);
        } break;
        default: {
            // logWarn("Unknown SDL event: {}", ev.type);
        } break;
    }

    return SDL_APP_CONTINUE;
}

extern "C" SDL_AppResult SDL_AppIterate(void* appstate)
{
#ifdef BLOCKGAME_ENABLE_TRACY
    ZoneScopedN("AppIterate");
#endif
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    auto& app                = *static_cast<AppContext*>(appstate);
    auto& game               = *app.game;
    auto const previous_time = app.current_time;
    app.current_time         = bg::get_time();
    assertr(app.current_time.count() > 0);

    auto& pendingActions = game.pendingActions;

    try {
        [[maybe_unused]] static bool _ = [&app]() {
            auto const test_callback = []() { logInfo("Button pressed!"); };
            bg::Button button{ test_callback };
            SDL_FRect rect{ 100, 100, 100, 100 };
            bg::VisibleButton visible_button{ button, rect };
            app.game->gui_objects.add(std::move(visible_button));
            bg::TextButton text_button{ bg::Text{ "test" }, bg::VisibleButton{ bg::Button{ test_callback },
                                                                               SDL_FRect{ 200, 200, 50, 50 } } };
            text_button.set_pos(SDL_Point{ 500, 500 });
            app.game->gui_objects.add(std::move(text_button));
            return true;
        }();

        static bool gameInputManagerInitialized = false;
        if (!gameInputManagerInitialized) {
            app.game->input_manager.registerCallbackOnRelease(
                SDLK_F1, [&app]() { app.game->ingameHudVisible = !app.game->ingameHudVisible; });

            app.game->input_manager.registerCallbackOnRelease(
                SDLK_F3, [&app]() { app.game->f3_window_on = !app.game->f3_window_on; });

            app.game->input_manager.registerCallbackOnRelease(SDLK_F4, [&app]() {
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

            app.game->input_manager.registerCallbackOnRelease(
                SDLK_F6, [&app]() { app.game->player.setFlying(!app.game->player.isFlying()); });

            app.game->input_manager.registerCallbackOnRelease(
                SDLK_F7, [&app]() { app.game->player.setNoclip(!app.game->player.isNoclip()); });

            app.game->input_manager.registerCallbackOnRelease(SDLK_F11,
                                                              [&app]() { toggleWindowFullscreen(app.window); });

            app.game->input_manager.registerCallbackOnHold(SDLK_LSHIFT, [&app]() { app.game->fastNoClipSpeed = true; });

            app.game->input_manager.registerCallbackOnRelease(SDLK_LSHIFT,
                                                              [&app]() { app.game->fastNoClipSpeed = false; });

            app.game->input_manager.registerCallbackOnRelease(SDLK_Q, [&app]() {
                // Debug
                logDebug("Player Pos: {}", glm::to_string(app.game->player.getPos(1.0)));
            });

            app.game->input_manager.registerCallbackOnRelease(
                SDL_BUTTON_LEFT, [&app, &pendingActions]() { pendingActions.push_back(SwingAction{}); });

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
        bg::Inputs curr_inputs = app.game->prev_inputs;

        // TODO: The first time prev_inputs is set, it should be set from the player inputs, otherwise if you hold
        // a key when the game is starting it won't be registered.
        bool mouse_updated = false;
        while (auto input_opt = app.game->input_event_buffer->consume()) {
            auto& input            = input_opt.value();
            static int input_count = 0;
            auto& text_button      = std::get<bg::TextButton>(app.game->gui_objects.objects[1]);
            text_button.update_text_str(std::to_string(input_count));
            ++input_count;
            // logDebug("Received event. Event type: {}", input.type);

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
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_Q)) {
                if (!app.game->prev_inputs.q) {
                    app.game->input_manager.queueInputOnDown(SDLK_Q);
                }
                app.game->input_manager.queueInputOnHold(SDLK_Q);
                curr_inputs.q = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_Q)) {
                app.game->input_manager.queueInputOnRelease(SDLK_Q);
                curr_inputs.q = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_F1)) {
                if (!app.game->prev_inputs.f1) {
                    app.game->input_manager.queueInputOnDown(SDLK_F1);
                }
                app.game->input_manager.queueInputOnHold(SDLK_F1);
                curr_inputs.f1 = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_F1)) {
                app.game->input_manager.queueInputOnRelease(SDLK_F1);
                curr_inputs.f1 = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_F3)) {
                if (!app.game->prev_inputs.f3) {
                    app.game->input_manager.queueInputOnDown(SDLK_F3);
                }
                app.game->input_manager.queueInputOnHold(SDLK_F3);
                curr_inputs.f3 = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_F3)) {
                app.game->input_manager.queueInputOnRelease(SDLK_F3);
                curr_inputs.f3 = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_F4)) {
                if (!app.game->prev_inputs.f4) {
                    app.game->input_manager.queueInputOnDown(SDLK_F4);
                }
                app.game->input_manager.queueInputOnHold(SDLK_F4);
                curr_inputs.f4 = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_F4)) {
                app.game->input_manager.queueInputOnRelease(SDLK_F4);
                curr_inputs.f4 = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_F6)) {
                if (!app.game->prev_inputs.f6) {
                    app.game->input_manager.queueInputOnDown(SDLK_F6);
                }
                app.game->input_manager.queueInputOnHold(SDLK_F6);
                curr_inputs.f6 = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_F6)) {
                app.game->input_manager.queueInputOnRelease(SDLK_F6);
                curr_inputs.f6 = false;
            } else if ((input.type == SDL_EVENT_KEY_DOWN) && (input.key.key == SDLK_F7)) {
                if (!app.game->prev_inputs.f7) {
                    app.game->input_manager.queueInputOnDown(SDLK_F7);
                }
                app.game->input_manager.queueInputOnHold(SDLK_F7);
                curr_inputs.f7 = true;
            } else if ((input.type == SDL_EVENT_KEY_UP) && (input.key.key == SDLK_F7)) {
                app.game->input_manager.queueInputOnRelease(SDLK_F7);
                curr_inputs.f7 = false;
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

        // Process input
        app.game->input_manager.processQueues();

        if (curr_inputs.a == true) {
            pendingActions.push_back(MoveAction{ MoveAction::Direction::LEFT });
        }
        if (curr_inputs.d == true) {
            pendingActions.push_back(MoveAction{ MoveAction::Direction::RIGHT });
        }
        if (curr_inputs.w == true) {
            pendingActions.push_back(MoveAction{ MoveAction::Direction::FORWARD });
        }
        if (curr_inputs.s == true) {
            pendingActions.push_back(MoveAction{ MoveAction::Direction::BACKWARD });
        }
        if (curr_inputs.lctrl == true) {
            pendingActions.push_back(MoveAction{ MoveAction::Direction::DOWN });
        }
        if (curr_inputs.space == true) {
            pendingActions.push_back(MoveAction{ MoveAction::Direction::UP });
        }

        // Process game state

        auto calculateNewYaw = [](float xRel, float yaw, float mouseSensitivity) {
            return yaw + (xRel * mouseSensitivity);
        };

        auto calculateNewPitch = [](float yRel, float pitch, float mouseSensitivity) {
            return std::clamp(pitch - (yRel * mouseSensitivity), -89.0f, 89.0f);
        };

        if (!game.debug_window_on) {
            // Update camera
            constexpr float mouseSensitivity{ 0.7 };
            if (mouse_updated) {
                game.player.currentCamera->setYaw(
                    calculateNewYaw(curr_inputs.xRel, game.player.currentCamera->getYaw(), mouseSensitivity));
                game.player.currentCamera->setPitch(
                    calculateNewPitch(curr_inputs.yRel, game.player.currentCamera->getPitch(), mouseSensitivity));
            }

            // Note:
            // The camera front must be calculated before movement because camera front is used in movement
            // calculations.
            game.player.currentCamera->updateFront(
                bg::cameraFront(game.player.currentCamera->getYaw(), game.player.currentCamera->getPitch()));
        }

        std::chrono::nanoseconds timeSinceLastTick = bg::get_time() - app.game->prevTickTimestamp;

        auto const timeBetweenTicks = std::chrono::milliseconds{ 30 }; // 40 ticks per second

        if ((app.game->prevTickTimestamp != std::chrono::nanoseconds{ 0 })) {
            if (timeSinceLastTick >= (timeBetweenTicks * 1.5)) {
                logWarn("Time since last tick was long: {}",
                        std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastTick));
            }
        }

        bool swung         = false;
        bool moved         = false;
        auto movedBitfield = MoveAction::Direction::NONE;

        {
#ifdef BLOCKGAME_ENABLE_TRACY
            ZoneScopedN("GameLogic");
#endif
            // TODO: Process multiple ticks if behind.
            // 20 ticks / second == 50 ms between ticks
            if (timeSinceLastTick >= timeBetweenTicks) {
                ++app.game->tick;

                game.player.updatePos(game.player.getPos(1.0));

                // Load chunks, generating as needed.
                constexpr std::uint64_t worldSeed = 123;
                game.chunkManager.updateChunks(game.player.getPos(1.0), game.chunkLoadRadius, worldSeed);

                // Process game logic.
                if (!game.debug_window_on) {
                    for (auto&& action : pendingActions) {
                        std::visit(bg::overloaded{
                                       [&](bg::SwingAction&) {
                                           if (!swung) {
                                               handleSwingAction(game);
                                               swung = true;
                                           }
                                       },
                                       [&](bg::MoveAction& action) {
                                           // TODO: Allowing multiple actions means moving diagonally is faster than in
                                           // cardinal directions. Instead, the full move action bitwise should be
                                           // passed to handleMoveAction and it should be normalized there.
                                           if ((movedBitfield & action.direction) == 0) {
                                               handleMoveAction(game, action);
                                               movedBitfield =
                                                   static_cast<MoveAction::Direction>(movedBitfield | action.direction);
                                           }
                                       },
                                   },
                                   action);
                    }
                }

                game.player.tick();

                pendingActions.clear();

                app.game->prevTickTimestamp = bg::get_time();
            }
        }

        app.game->prev_inputs = curr_inputs;

        // Rendering

        // Ratio of progress to the next tick.
        timeSinceLastTick = bg::get_time() - app.game->prevTickTimestamp;
        float partialTick =
            static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(timeSinceLastTick).count()) /
            static_cast<float>(timeBetweenTicks.count());
        // Clamp to [0.0, 1.0] to be safe.
        partialTick = std::clamp(partialTick, 0.0f, 1.0f);

        // logDebug("Progress to next tick: {:2.2f}%", partialTick);
        {
#ifdef BLOCKGAME_ENABLE_TRACY
            ZoneScopedN("Render");
#endif

            auto& imguiIO = ImGui::GetIO();
            if (game.f3_window_on) {
                imguiIO.NavActive = false;
                ImGui::Begin("F3 Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                auto const chunkManagerStats = game.chunkManager.getStats();
                // clang-format off
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / imguiIO.Framerate, imguiIO.Framerate);
                ImGui::Text(" ");
                ImGui::Text("%s", std::format("Pos: {}", glm::to_string(game.player.getPos(partialTick))).c_str());
                ImGui::Text("%s", std::format("Angle: {}", glm::to_string(game.player.currentCamera->getFront(partialTick))).c_str());
                ImGui::Text("%s", std::format("Angle (degrees): {}", glm::to_string(game.player.currentCamera->getFront(partialTick))).c_str());
                ImGui::Text("%s", std::format("Velocity: {}", glm::to_string(game.player.getVelocity())).c_str());
                ImGui::Text("%s", std::format("Velocity Magnitude: {}", glm::length(game.player.getVelocity())).c_str());
                ImGui::Text("%s", std::format("Grounded: {}", game.player.isGrounded()).c_str());
                ImGui::Text("%s", std::format("Flying: {}", game.player.isFlying()).c_str());
                ImGui::Text("%s", std::format("Noclip: {}", game.player.isNoclip()).c_str());
                ImGui::Text(" ");
                ImGui::Text("%s", std::format("Loaded Chunk Count: {}", chunkManagerStats.loadedChunkCount).c_str());
                // clang-format on
                ImGui::End();
            }

            if (game.debug_window_on) {
                imguiIO.NavActive = true;
                ImGui::ShowDemoWindow();
            } else {
                imguiIO.NavActive = false;
            }

            bg::Renderer::render(app, partialTick);
        }

        ++app.game->frame_num;

        // Limit fps to 300
        auto const fps = bg::calculate_fps(previous_time, bg::get_time());
        if (fps > 300) {
            SDL_Delay(1);
        }
    } catch (const std::exception& e) {
        logInfo("{}", e.what());
        app.app_quit = SDL_APP_FAILURE;
    }

    return app.app_quit;
}

extern "C" void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    static_cast<void>(result);

    logInfo("Application quitting...");
    auto* app = static_cast<AppContext*>(appstate);
    delete app;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    bg::destroy_global_font();

    SDL_Quit();
    logInfo("Application quit.");
}

} // namespace bg
