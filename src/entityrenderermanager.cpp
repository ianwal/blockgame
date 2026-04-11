#include "src/entityrenderermanager.h"

#include "src/assertr.h"
#include "src/entityrenderer.h"

#include <map>
#include <memory>

namespace bg
{

void EntityRendererManager::addRenderer(EntityType entityType, std::unique_ptr<IEntityRenderer> renderer)
{
    assertr(!mEntityRendererMap.contains(entityType));
    mEntityRendererMap[entityType] = std::move(renderer);
}

void EntityRendererManager::renderEntity(IEntity const& entity, float partialTick) const
{
    auto it = mEntityRendererMap.find(entity.getEntityType());
    assertr(it != mEntityRendererMap.end());
    it->second->render(entity, partialTick);
}

} // namespace bg
