#include "src/time.h"

namespace Alf
{

std::chrono::milliseconds get_time()
{
    return std::chrono::milliseconds{SDL_GetTicks()};
}

void test()
{
    Alf::get_time();
}

} // namespace Alf
