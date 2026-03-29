#pragma once

#include "src/app_context.h"
#include "src/time.h"

#include <GL/glew.h>

namespace alf
{

class Renderer
{
public:
    static void init();

    static void cleanup();

    static void render(AppContext& app_ctx);

private:
    static GLuint VertexArrayID;
};

std::int64_t calculate_fps(Timestamp previous_frame, Timestamp current_frame);

} // namespace alf
