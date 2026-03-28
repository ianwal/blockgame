#pragma once

#include "src/button.h"
#include "src/event_handling.h"
#include "src/time.h"
#include <cassert>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <variant>

namespace Alf
{

using Render_layer_t = uint8_t;

using Gui_object_var_t = std::variant<Alf::VisibleButton, Alf::TextButton, Alf::Text>;

struct GuiObjects
{
    void add(Gui_object_var_t &&obj)
    {
        objects.push_back(obj);
    }

    void render(SDL_Renderer &renderer)
    {
        for (auto &obj : this->objects)
        {
            std::visit([&renderer](auto &&arg) { arg.render(renderer); }, obj);
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
    enum class EntityType
    {
        BLOCK,
    };

    virtual EntityType getEntityType() const = 0;

    virtual ~IEntity() = default;
};

/// Interface for an entity renderer.
class IEntityRenderer
{
  public:
    virtual void render(IEntityRenderInfo const &renderInfo) = 0;

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

    void renderEntity(IEntity const &entity) const
    {
        auto it = mEntityRendererMap.find(entity.getEntityType());
        assert(it != mEntityRendererMap.end());
        it->second->render({});
    }

  private:
    std::map<IEntity::EntityType, std::unique_ptr<IEntityRenderer>> mEntityRendererMap{};
};

struct GameContext
{
    std::vector<IEntity> entities{};

    EntityRendererManager entityRendererManager{};

    GuiObjects gui_objects{};
    std::unique_ptr<EventBuffer> input_event_buffer{std::make_unique<EventBuffer>()};
    uint64_t frame_num{};
    Timestamp previous_frame_time{};
};

} // namespace Alf
