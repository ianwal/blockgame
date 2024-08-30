#pragma once

#include <SDL3/SDL.h>
#include <cassert>
#include <chrono>

namespace Alf
{

using Timestamp = std::chrono::nanoseconds;

// Get monotonic time.
std::chrono::nanoseconds get_time();

void test();

} // namespace Alf
