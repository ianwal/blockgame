#include "src/button.h"

namespace bg
{

namespace
{

void render_text(SDL_Renderer& renderer, Text const& text)
{
    SDL_Surface* text_surface{ TTF_RenderText_Solid(&get_font(), text.get_string().c_str(), 0, text.color) };
    SDL_Texture* text_texture{ SDL_CreateTextureFromSurface(&renderer, text_surface) };
    SDL_FRect const render_quad{ static_cast<float>(text.pos.x), static_cast<float>(text.pos.y),
                                 static_cast<float>(text_surface->w), static_cast<float>(text_surface->h) };
    SDL_RenderTexture(&renderer, text_texture, nullptr, &render_quad);
    SDL_DestroySurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

SDL_Rect frect_to_rect(SDL_FRect frect)
{
    return SDL_Rect{ static_cast<int>(frect.x), static_cast<int>(frect.y), static_cast<int>(frect.w),
                     static_cast<int>(frect.h) };
}
} // namespace

SDL_Point get_center_in_rect(SDL_Rect rect)
{
    int const x{ rect.x + (rect.w / 2) };
    int const y{ rect.y + (rect.h / 2) };
    return SDL_Point{ x, y };
}

SDL_Point get_center_in_rect(SDL_FRect frect)
{
    return get_center_in_rect(frect_to_rect(frect));
}

Dim2d get_text_size(Text const& text)
{
    Dim2d dim{};
    auto const result = TTF_GetStringSize(text.font.font, text.get_string().c_str(), 0, &dim.width, &dim.height);
    assert(result == true);
    return dim;
}

void center_text(Text& text, SDL_Point const point)
{
    auto const text_size = get_text_size(text);
    text.pos.x           = point.x - (text_size.width / 2);
    text.pos.y           = point.y - (text_size.height / 2);
}

void Text::render(SDL_Renderer& renderer) const
{
    render_text(renderer, *this);
}

} // namespace bg
