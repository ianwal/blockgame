#pragma once

#include <SDL3/SDL.h>
#include <mutex>
#include <optional>

namespace Alf
{

class EventBuffer
{
  public:
    // Read and set the read input to empty.
    // May return empty.
    std::optional<SDL_Event> consume()
    {
        std::optional<SDL_Event> result{};
        {
            std::scoped_lock{this->event_mtx};
            if (this->event.has_value())
            {
                result = this->event;
                this->event = std::nullopt;
            }
        }
        return result;
    }

    // Write an event to the buffer.
    void write(SDL_Event event)
    {
        std::scoped_lock{this->event_mtx};
        this->event = event;
    }

  private:
    std::optional<SDL_Event> event{std::nullopt};
    std::mutex event_mtx{};
};

} // namespace Alf
