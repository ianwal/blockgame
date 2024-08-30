#pragma once

#include "src/common.h"
#include "src/font.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cassert>
#include <functional>
#include <optional>
#include <string>

namespace Alf
{

SDL_Point get_center_in_rect(SDL_Rect rect);

SDL_Point get_center_in_rect(SDL_FRect frect);

using ButtonCallbackFn = std::function<void(void)>;

class Button
{
  public:
    Button(ButtonCallbackFn callback_fn) : m_callback_fn{std::move(callback_fn)}
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
        if (m_callback_fn.has_value())
        {
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
    VisibleButton(Button button, SDL_FRect rect) : button(std::move(button)), rect{rect}
    {
    }

    void render(SDL_Renderer &renderer) const
    {
        SDL_SetRenderDrawColor(&renderer, 100, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderRect(&renderer, &rect);
        auto const result = SDL_RenderFillRect(&renderer, &rect);
        assert(result == 0);
    }

    Button button;
    SDL_FRect rect;
};

class Text
{
  public:
    Text(std::string text) : Text{text, SDL_Point{}}
    {
    }

    Text(std::string text, SDL_Point pos) : text{std::move(text)}, pos{pos}
    {
    }

    void render(SDL_Renderer &renderer) const
    {
        SDL_Color const forecol{0, 0, 0, 255};
        SDL_Surface *text_surface{TTF_RenderUTF8_Solid(&get_font(), text.c_str(), forecol)};
        SDL_Texture *text_texture{SDL_CreateTextureFromSurface(&renderer, text_surface)};
        SDL_FRect const render_quad{static_cast<float>(pos.x), static_cast<float>(pos.y),
                                    static_cast<float>(text_surface->w), static_cast<float>(text_surface->h)};
        SDL_RenderTexture(&renderer, text_texture, nullptr, &render_quad);
        SDL_DestroySurface(text_surface);
        SDL_DestroyTexture(text_texture);
    }

    std::string text;
    SDL_Point pos;
    Font font{};
};

// Get the size of the rendered text.
Dim2d get_text_size(Text const &text);

// Center the text at the point.
void center_text(Text &text, SDL_Point const point);

class TextButton
{
  public:
    TextButton(Text text, VisibleButton visible_button)
        : text{std::move(text)}, visible_button{std::move(visible_button)}
    {
        center_text(this->text, get_center_in_rect(this->visible_button.rect));
    }

    void render(SDL_Renderer &renderer) const
    {
        visible_button.render(renderer);
        text.render(renderer);
    }

    Text text;
    VisibleButton visible_button;
};

} // namespace Alf
