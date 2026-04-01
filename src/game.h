#pragma once

#include "src/actions.h"
#include "src/block.h"
#include "src/button.h"
#include "src/camera.h"
#include "src/event_handling.h"
#include "src/game.h"
#include "src/input_manager.h"
#include "src/player.h"
#include "src/time.h"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <cassert>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

namespace alf
{

using Render_layer_t = std::uint8_t;

using Gui_object_var_t = std::variant<alf::VisibleButton, alf::TextButton, alf::Text>;

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

/// Interface for entity rendering information.
class IEntityRenderInfo
{
public:
    virtual ~IEntityRenderInfo() = default;
};

/// Interface for entities.
class IEntity
{
public:
    enum class EntityType {
        BLOCK,
    };

    virtual EntityType getEntityType() const = 0;

    virtual ~IEntity() = default;
};

/// Interface for an entity renderer.
class IEntityRenderer
{
public:
    virtual void render(IEntityRenderInfo const& renderInfo) = 0;

    virtual ~IEntityRenderer() = default;
};

class EntityRendererManager
{
public:
    EntityRendererManager() = default;

    void addRenderer(IEntity::EntityType entityType, std::unique_ptr<IEntityRenderer> renderer)
    {
        assert(!mEntityRendererMap.contains(entityType));
        mEntityRendererMap[entityType] = std::move(renderer);
    }

    void renderEntity(IEntity const& entity) const
    {
        auto it = mEntityRendererMap.find(entity.getEntityType());
        assert(it != mEntityRendererMap.end());
        it->second->render({});
    }

private:
    std::map<IEntity::EntityType, std::unique_ptr<IEntityRenderer>> mEntityRendererMap{};
};

/// Inputs. True is pressed down, false is not pressed.
struct Inputs {
    bool mouse1{};

    bool w{};
    bool a{};
    bool s{};
    bool d{};
    bool z{};
    bool f3{};
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

struct GameContext {
    GameContext();

    std::vector<IEntity> entities{};

    EntityRendererManager entityRendererManager{};

    Player player{};

    std::vector<Block> blocks{};

    Inputs prev_inputs{};

    bool debug_window_on = false;

    float fastNoClipSpeed = false;

    GuiObjects gui_objects{};
    std::unique_ptr<EventBuffer> input_event_buffer{ std::make_unique<EventBuffer>() };
    InputManager input_manager{};
    std::uint64_t frame_num{};
    Timestamp previous_frame_time{};
};

void handleSwingAction(GameContext& game);

void handleMoveAction(GameContext& game, MoveAction const& action);

} // namespace alf
