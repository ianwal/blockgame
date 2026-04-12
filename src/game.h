#pragma once

#include "src/actions.h"
#include "src/assertr.h"
#include "src/block.h"
#include "src/button.h"
#include "src/camera.h"
#include "src/chunk.h"
#include "src/entity.h"
#include "src/entityrenderermanager.h"
#include "src/event_handling.h"
#include "src/game.h"
#include "src/input_manager.h"
#include "src/levelmanager.h"
#include "src/player.h"
#include "src/playerinventory.h"
#include "src/time.h"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <cassert>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace bg
{

using Render_layer_t = std::uint8_t;

using Gui_object_var_t = std::variant<bg::VisibleButton, bg::TextButton, bg::Text>;

struct GuiObjects {
    void add(Gui_object_var_t&& obj)
    {
        objects.push_back(obj);
    }

    void render(SDL_Renderer& renderer)
    {
        for (auto& obj : this->objects) {
            std::visit([&renderer](auto&& arg) { arg.render(renderer); }, obj);
        }
    }

    std::deque<Gui_object_var_t> objects{};
};

/// Inputs. True is pressed down, false is not pressed.
struct Inputs {
    bool mouse1{};
    bool mouse2{};

    bool num1{};
    bool num2{};
    bool num3{};

    bool w{};
    bool a{};
    bool s{};
    bool d{};
    bool z{};
    bool q{};
    bool f{};
    bool f1{};
    bool f3{};
    bool f4{};
    bool f6{};
    bool f7{};
    bool f11{};
    bool lshift{};
    bool space{};
    bool lctrl{};
    bool esc{};

    float x{}; // Mouse X coordinate
    float y{}; // Mouse Y coordinate

    float xRel{}; // Mouse X coordinate relative motion
    float yRel{}; // Mouse Y coordinate relative motion

    std::string to_string() const
    {
        std::stringstream ss{};
        ss << "[w: " << w;
        ss << ", a: " << a;
        ss << ", s: " << s;
        ss << ", d: " << d;
        ss << ", sp: " << space;
        ss << ", lctrl: " << lctrl;
        ss << "]";
        return ss.str();
    }
};

inline std::string to_str(glm::vec3 const& v)
{
    std::stringstream ss{};
    ss << std::fixed << std::setprecision(2);
    ss << "[x: " << v.x;
    ss << ", y: " << v.y;
    ss << ", z: " << v.z;
    ss << "]";
    return ss.str();
}

/// If an entity has fallen out of the world.
constexpr bool isOutOfWorld(glm::vec3 const& pos)
{
    return pos.y < 0.0f;
}

struct GameContext {
    GameContext();

    EntityRendererManager entityRendererManager{};

    Player player{};

    Inputs prev_inputs{};

    bool f3_window_on    = false;
    bool debug_window_on = false;
    bool ingameHudVisible{ true };

    float fastNoClipSpeed = false;

    ChunkManager chunkManager{};

    std::int32_t chunkLoadRadius{ 2 };
    std::int32_t chunkRenderDistance = chunkLoadRadius;

    LevelManager levelManager{};

    PlayerInventory playerInventory{};

    std::unique_ptr<EventBuffer> input_event_buffer{ std::make_unique<EventBuffer>() };
    InputManager input_manager{};
    std::vector<InputAction> pendingActions{};
    std::uint64_t tick{};
    std::uint64_t frame_num{};
    Timestamp prevTickTimestamp{};
};

/// Global game singleton.
class GlobalGame
{
public:
    static GameContext& get();

    static void set(GameContext* newGame);

private:
    static GameContext* game;
};

/// Get the block that the player is currently looking at, if there is one.
std::optional<Block> getBlockPlayerIsLookingAt(ChunkManager const& chunkManager, Player const& player);

void handleSwingAction(GameContext& game);

void handleMoveAction(GameContext& game, MoveAction const& action);

void handlePlaceAction(GameContext& game);

void handleDropItemAction(GameContext& game);

void handleChangeHotbarPosAction(GameContext& game, ChangeHotbarPosAction const& action);

} // namespace bg
