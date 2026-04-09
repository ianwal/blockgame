#pragma once

#include <SDL3_mixer/SDL_mixer.h>

#include <cstdint>
#include <map>
#include <span>
#include <string>

namespace bg
{

/// Loads audio files.
class AudioResourceManager
{
public:
    AudioResourceManager() = default;

    /// Get the audio by name.
    std::span<std::uint8_t> get(std::string const& name);

    /// Load the audio and assign it a name.
    AudioResourceManager& load(std::string const& name, std::string const& path);

    void cleanup();

    // Not copyable or moveable.
    AudioResourceManager(AudioResourceManager const&)            = delete;
    AudioResourceManager(AudioResourceManager&&)                 = delete;
    AudioResourceManager& operator=(AudioResourceManager const&) = delete;
    AudioResourceManager& operator=(AudioResourceManager&&)      = delete;

    ~AudioResourceManager();

private:
    std::map<std::string, std::span<std::uint8_t>> audioFiles{};
};

void playSound2d(std::string const& sound);

} // namespace bg
