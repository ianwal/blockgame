#include "src/time.h"

namespace alf
{

std::chrono::nanoseconds get_time()
{
    return std::chrono::steady_clock::now().time_since_epoch();
}

} // namespace alf
