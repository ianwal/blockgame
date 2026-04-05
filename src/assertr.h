#pragma once

#include <source_location>
#include <string_view>

namespace bg
{

inline static constexpr bool config_asserts_enabled = true;

void assertr(bool condition, std::source_location const& location = std::source_location::current());

void assertr(bool condition, std::string_view msg,
             std::source_location const& location = std::source_location::current());

} // namespace bg
