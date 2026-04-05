#pragma once

#include <SDL3/SDL.h>

#include <format>
#include <utility>

namespace bg
{

template <class... Args>
static void logCritical(std::format_string<Args...> fmt, Args&&... args)
{
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", std::format(fmt, std::forward<Args>(args)...).c_str());
}

template <class... Args>
static void logError(std::format_string<Args...> fmt, Args&&... args)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", std::format(fmt, std::forward<Args>(args)...).c_str());
}

template <class... Args>
static void logWarn(std::format_string<Args...> fmt, Args&&... args)
{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s", std::format(fmt, std::forward<Args>(args)...).c_str());
}

template <class... Args>
static void logInfo(std::format_string<Args...> fmt, Args&&... args)
{
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", std::format(fmt, std::forward<Args>(args)...).c_str());
}

template <class... Args>
static void logDebug(std::format_string<Args...> fmt, Args&&... args)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", std::format(fmt, std::forward<Args>(args)...).c_str());
}

} // namespace bg
