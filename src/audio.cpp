#include "src/audio.h"

#include "src/app_context.h"
#include "src/assertr.h"
#include "src/logger.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <cstdint>
#include <span>
#include <string>

namespace bg
{

std::span<std::uint8_t> AudioResourceManager::get(std::string const& name)
{
    assertr(audioFiles.contains(name), name);
    return audioFiles.at(name);
}

AudioResourceManager& AudioResourceManager::load(std::string const& name, std::string const& path)
{
    assertr(!audioFiles.contains(name));
    if (!audioFiles.contains(name)) {
        std::size_t size{};
        std::uint8_t* loadedAudio = static_cast<std::uint8_t*>(SDL_LoadFile(path.c_str(), &size));
        assertr(loadedAudio != nullptr);
        audioFiles[name] = std::span{ loadedAudio, size };
    }
    return *this;
}

void AudioResourceManager::cleanup()
{
    for (auto& a : audioFiles) {
        SDL_free(a.second.data());
    }
    audioFiles.clear();
}

AudioResourceManager::~AudioResourceManager()
{
    cleanup();
}

void playSound2d(std::string const& sound)
{
    auto soundFile   = App::get().audioResourceManager.get(sound);
    MIX_Audio* audio = MIX_LoadAudioNoCopy(App::get().mixer, soundFile.data(), soundFile.size_bytes(), true);
    assertr(audio != nullptr);
    MIX_SetTrackAudio(App::get().track, audio);
    bool const playResult = MIX_PlayTrack(App::get().track, {});
    if (!playResult) {
        logWarn("Failed to play track for sound: {}", sound);
    }
}

} // namespace bg
