#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace bg
{

using EntityId = std::uint32_t;

enum class EntityType : std::uint8_t {
    DROPPED_ITEM = 0,
};

/// Interface for entities.
class IEntity
{
public:
    IEntity() = default;

    void setId(EntityId id);

    [[nodiscard]] EntityId getId() const;

    virtual void tick() = 0;

    [[nodiscard]] virtual glm::vec3 getPos(float partialTick) const = 0;

    /// Get rotation as <pitch, yaw, roll>
    [[nodiscard]] virtual glm::vec3 getRotation(float partialTick) const = 0;

    [[nodiscard]] virtual EntityType getEntityType() const = 0;

    /// If the entity must be saved when it's unloaded.
    [[nodiscard]] virtual bool isPersistent() const = 0;

    virtual void setDead() = 0;

    [[nodiscard]] virtual bool isDead() const = 0;

    virtual ~IEntity() = default;

private:
    EntityId id{};
};

} // namespace bg
