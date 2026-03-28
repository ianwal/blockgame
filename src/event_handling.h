#pragma once

#include <SDL3/SDL.h>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace Alf
{

/// A thread-safe event queue.
class EventBuffer
{
public:
    /// Pop an element from the queue.
    ///
    /// May return empty.
    std::optional<SDL_Event> consume()
    {
        {
            std::scoped_lock lock{ this->event_mtx };
            if (!this->event_queue.empty()) {
                auto result = std::move(this->event_queue.front());
                this->event_queue.pop();
                return result;
            } else {
                return std::nullopt;
            }
        }
    }

    /// Push an event to the buffer.
    void push(SDL_Event event)
    {
        std::scoped_lock lock{ this->event_mtx };
        event_queue.push(std::move(event));
    }

private:
    std::queue<SDL_Event> event_queue{};
    std::mutex event_mtx{};
};

} // namespace Alf
