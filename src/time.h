#pragma once

#include <SDL3/SDL.h>

#include <cassert>
#include <chrono>

namespace alf
{

using Timestamp = std::chrono::nanoseconds;

// Get monotonic time.
std::chrono::nanoseconds get_time();

} // namespace alf
