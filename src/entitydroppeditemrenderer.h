#pragma once

#include "src/app_context.h"
#include "src/entitydroppeditem.h"
#include "src/entityrenderer.h"

#include <GL/glew.h>

namespace bg
{

class EntityDroppedItemRenderer : public IEntityRenderer
{
public:
    EntityDroppedItemRenderer();

    void render(IEntity const& ientity, float partialTick) override;

    ~EntityDroppedItemRenderer();

private:
    GLuint vao;
    GLuint vertexbuffer;
    GLuint shaderId;
    GLuint matrixId;
    GLuint vbo;
};

} // namespace bg
