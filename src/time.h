#pragma once

#include <SDL3/SDL.h>
#include <cassert>
#include <chrono>

namespace Alf
{

using Timestamp = std::chrono::milliseconds;

// Get monotonic time.
std::chrono::milliseconds get_time();

void test();

} // namespace Alf
