#pragma once

#include "src/entity.h"
#include "src/entityrenderer.h"

#include <map>
#include <memory>

namespace bg
{

class EntityRendererManager
{
public:
    EntityRendererManager() = default;

    void addRenderer(EntityType entityType, std::unique_ptr<IEntityRenderer> renderer);

    void renderEntity(IEntity const& entity, float partialTick) const;

private:
    std::map<EntityType, std::unique_ptr<IEntityRenderer>> mEntityRendererMap{};
};

} // namespace bg
