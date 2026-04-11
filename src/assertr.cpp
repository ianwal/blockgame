#include "src/assertr.h"

#include "src/logger.h"

#include <cstdlib>
#include <exception>
#include <format>
#include <source_location>
#include <string_view>

namespace bg
{

void assertr(bool condition, std::source_location const& location)
{
    if constexpr (config_asserts_enabled) {
        if (!condition) {
            logCritical("Assert \033[0;31mFAILED\033[0m in file {} on line {}", location.file_name(), location.line());
            if (config_asserts_crash) {
                std::abort();
            }
        }
    }
}

void assertr(bool condition, std::string_view msg, std::source_location const& location)
{
    if constexpr (config_asserts_enabled) {
        if (!condition) {
            logCritical("Assert \033[0;31mFAILED\033[0m in file {} on line {}. Message: {}", location.file_name(),
                        location.line(), msg);
            if (config_asserts_crash) {
                std::abort();
            }
        }
    }
}

} // namespace bg
