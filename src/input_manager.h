#pragma once

#include <SDL3/SDL.h>
#include <functional>
#include <map>
#include <vector>

class InputManager
{
public:
    /// Register a callback to call on the first tick a button is pressed down.
    void registerCallbackOnDown(SDL_Keycode key, std::function<void()> callback)
    {
        onDownMap.insert({ key, callback });
    }

    /// Register a callback to call once when a button is pressed and released.
    void registerCallbackOnRelease(SDL_Keycode key, std::function<void()> callback)
    {
        onReleaseMap.insert({ key, callback });
    }

    /// Register a callback to call every tick a button is currently held.
    void registerCallbackOnHold(SDL_Keycode key, std::function<void()> callback)
    {
        onHoldMap.insert({ key, callback });
    }

    /// Push an input on release (does not call callbacks until processQueues() is called).
    void queueInputOnDown(SDL_Keycode key)
    {
        onDownQueue.push_back(key);
    }

    /// Push an input on release (does not call callbacks until processQueues() is called).
    void queueInputOnRelease(SDL_Keycode key)
    {
        onReleaseQueue.push_back(key);
    }

    /// Push an input on hold (does not call callbacks until processQueues() is called).
    void queueInputOnHold(SDL_Keycode key)
    {
        onHoldQueue.push_back(key);
    }

    /// Process the input queues, calling any callbacks registered to pressed keys.
    void processQueues()
    {
        for (auto const& key : onDownQueue) {
            auto [begin, end] = onDownMap.equal_range(key);
            for (auto it = begin; it != end; ++it) {
                it->second();
            }
        }
        onDownQueue.clear();

        for (auto const& key : onReleaseQueue) {
            auto [begin, end] = onReleaseMap.equal_range(key);
            for (auto it = begin; it != end; ++it) {
                it->second();
            }
        }
        onReleaseQueue.clear();

        for (auto const& key : onHoldQueue) {
            auto [begin, end] = onHoldMap.equal_range(key);
            for (auto it = begin; it != end; ++it) {
                it->second();
            }
        }
        onHoldQueue.clear();
    }

    /// Clear all callbacks.
    void clear()
    {
        onDownMap.clear();
        onReleaseMap.clear();
        onHoldMap.clear();

        onDownQueue.clear();
        onReleaseQueue.clear();
        onHoldQueue.clear();
    }

private:
    std::multimap<SDL_Keycode, std::function<void()>> onDownMap{};
    std::multimap<SDL_Keycode, std::function<void()>> onReleaseMap{};
    std::multimap<SDL_Keycode, std::function<void()>> onHoldMap{};
    std::vector<SDL_Keycode> onDownQueue{};
    std::vector<SDL_Keycode> onReleaseQueue{};
    std::vector<SDL_Keycode> onHoldQueue{};
};
