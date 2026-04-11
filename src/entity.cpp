#include "src/entity.h"

#include <cstdint>

namespace bg
{

void IEntity::setId(EntityId id)
{
    this->id = id;
}

EntityId IEntity::getId() const
{
    return id;
}

} // namespace bg
