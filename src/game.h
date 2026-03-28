#pragma once

#include "src/button.h"
#include "src/event_handling.h"
#include "src/time.h"
#include <cassert>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

namespace Alf
{

using Render_layer_t = uint8_t;

using Gui_object_var_t = std::variant<Alf::VisibleButton, Alf::TextButton, Alf::Text>;

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

struct Coord3D {
    float x{};
    float y{};
    float z{};

    std::string to_string() const
    {
        std::stringstream ss{};
        ss << std::fixed << std::setprecision(2);
        ss << "[x: " << x;
        ss << ", y: " << y;
        ss << ", z: " << z;
        ss << "]";
        return ss.str();
    }
};

/// Inputs. True is pressed down, false is not pressed.
struct Inputs {
    bool w{};
    bool a{};
    bool s{};
    bool d{};
    bool space{};
    bool ctrl{};

    std::string to_string() const
    {
        std::stringstream ss{};
        ss << "[w: " << w;
        ss << ", a: " << a;
        ss << ", s: " << s;
        ss << ", d: " << d;
        ss << ", sp: " << space;
        ss << ", ctrl: " << ctrl;
        ss << "]";
        return ss.str();
    }
};

struct GameContext {
    std::vector<IEntity> entities{};

    EntityRendererManager entityRendererManager{};

    Coord3D player_position{ 4, 3, 3 }; // Player position in world space.

    Inputs prev_inputs{};

    GuiObjects gui_objects{};
    std::unique_ptr<EventBuffer> input_event_buffer{ std::make_unique<EventBuffer>() };
    uint64_t frame_num{};
    Timestamp previous_frame_time{};
};

} // namespace Alf
