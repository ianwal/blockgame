#pragma once

#include "src/aabb.h"
#include "src/entity.h"
#include "src/item.h"

#include <glm/glm.hpp>

namespace bg
{

class EntityDroppedItem : public IEntity
{
public:
    EntityDroppedItem() = default;

    virtual ~EntityDroppedItem() = default;

    void tick() override;

    [[nodiscard]] glm::vec3 getPos(float partialTick) const override;

    [[nodiscard]] glm::vec3 getRotation(float partialTick) const override;

    [[nodiscard]] EntityType getEntityType() const override;

    /// If the entity must be saved when it's unloaded.
    [[nodiscard]] bool isPersistent() const override;

    void setDead() override;

    [[nodiscard]] bool isDead() const override;

    //////

    bool isGrounded() const;

    void setVelocity(glm::vec3 newVel);

    [[nodiscard]] glm::vec3 getVelocity() const;

    [[nodiscard]] glm::vec3 getVelocity(float partialTick) const;

    void updatePos(glm::vec3 newPos);

    [[nodiscard]] AABB getAABB() const;

    void setItemType(ItemType itemType);

    [[nodiscard]] ItemType getItemType() const;

private:
    glm::vec3 pos{};         ///< Position.
    glm::vec3 prevPos = pos; ///< Position on the previous tick.
    glm::vec3 vel{};         ///< Velocity.
    glm::vec3 prevVel = vel; ///< Velocity on the previous tick.

    bool grounded{ false };
    bool dead{ false };

    static constexpr float gravityScale{ 0.04 };

    ItemType itemType{};
};

} // namespace bg
