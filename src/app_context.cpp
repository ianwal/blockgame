#include "src/app_context.h"

#include "src/common.h"
#include <SDL3/SDL.h>

#include <cassert>

namespace alf
{

Dim2d getWindowSize(AppContext& ctx)
{
    Dim2d dim{};
    auto get_window_size_status = SDL_GetWindowSize(ctx.window, &dim.width, &dim.height);
    assert(get_window_size_status);
    return dim;
}

} // namespace alf
