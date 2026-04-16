#pragma once

#include "src/app_context.h"
#include "src/time.h"

#include <GL/glew.h>

namespace bg
{

class Renderer
{
public:
    static void init();

    static void cleanup();

    static void render(AppContext& ctx, float partialTick);

private:
    static GLuint viewProjectionUboID;
};

std::int64_t calculate_fps(Timestamp previous_frame, Timestamp current_frame);

} // namespace bg
