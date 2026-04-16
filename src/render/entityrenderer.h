#pragma once

#include "src/entity.h"

namespace bg
{

/// Interface for an entity renderer.
class IEntityRenderer
{
public:
    virtual void render(IEntity const& entity, float partialTick) = 0;

    virtual ~IEntityRenderer() = default;
};

} // namespace bg
