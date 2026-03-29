#pragma once

#include "src/app_context.h"
#include "src/time.h"

namespace Alf
{

std::int64_t calculate_fps(Timestamp previous_frame, Timestamp current_frame);

void render(AppContext& app_ctx);

} // namespace Alf
