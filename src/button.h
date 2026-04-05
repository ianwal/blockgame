#pragma once

#include "src/common.h"
#include "src/font.h"
#include "src/shaders.h"

#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cassert>
#include <functional>
#include <optional>
#include <string>

namespace bg
{

SDL_Point get_center_in_rect(SDL_Rect rect);

SDL_Point get_center_in_rect(SDL_FRect frect);

using ButtonCallbackFn = std::function<void(void)>;

class Button
{
public:
    explicit Button(ButtonCallbackFn callback_fn) : m_callback_fn{ std::move(callback_fn) }
    {
    }

    void set_callback(ButtonCallbackFn callback_fn)
    {
        this->m_callback_fn = std::move(callback_fn);
    }

    void press()
    {
        pressed_callback();
    }

    void pressed_callback()
    {
        if (m_callback_fn.has_value()) {
            m_callback_fn.value()();
        }
    }

    SDL_Point m_pos;
    Dim2d m_dim;
    std::optional<ButtonCallbackFn> m_callback_fn;
};

class VisibleButton
{
public:
    VisibleButton(Button button, SDL_FRect rect) : button(std::move(button)), rect{ rect }
    {
    }

    void render(SDL_Renderer& renderer) const
    {
        SDL_SetRenderDrawColor(&renderer, 100, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(&renderer, &rect);
        auto const result = SDL_RenderFillRect(&renderer, &rect);
        assert(result);
    }

    Button button;
    SDL_FRect rect;
};

class Text
{
public:
    explicit Text(std::string str) : Text{ str, SDL_Point{} }
    {
    }

    Text(std::string str, SDL_Point pos) : str{ std::move(str) }, pos{ pos }
    {
    }

    void set_string(std::string str)
    {
        this->str = std::move(str);
    }

    std::string const& get_string() const
    {
        return this->str;
    }

    void render(SDL_Renderer& renderer) const;

    std::string str;
    SDL_Color color{ 0, 0, 0, 255 };
    SDL_Point pos;
    Font font{};
};

// Get the size of the rendered text.
Dim2d get_text_size(Text const& text);

// Center the text at the point.
void center_text(Text& text, SDL_Point const point);

class TextButton
{
public:
    TextButton(Text text, VisibleButton visible_button)
        : TextButton{ std::move(text), std::move(visible_button), SDL_Point{} }
    {
    }

    TextButton(Text text, VisibleButton visible_button, SDL_Point pos)
        : text{ std::move(text) }, visible_button{ std::move(visible_button) }
    {
    }

    void render(SDL_Renderer& renderer) const
    {
        this->visible_button.render(renderer);
        this->text.render(renderer);
    }

    void update_text_str(std::string str)
    {
        this->text.set_string(std::move(str));
        set_pos(pos);
    }

    void set_text(Text text)
    {
        this->text = text;
        // update the text position to the center of the rectangle.
        set_pos(pos);
    }

    void set_pos(SDL_Point new_pos)
    {
        this->pos             = new_pos;
        visible_button.rect.x = this->pos.x;
        visible_button.rect.y = this->pos.y;
        center_text(this->text, get_center_in_rect(this->visible_button.rect));
    }

    Text text;
    VisibleButton visible_button;
    SDL_Point pos;
};

} // namespace bg
