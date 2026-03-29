#pragma once

namespace alf
{

/// Dimension 2D.
struct Dim2d {
    int width{};
    int height{};
};

#define ASSETS_DIR "assets/"

#define FONT_FILE_NAME "roboto-regular.ttf"

#define FONTS_DIR ASSETS_DIR "fonts/"

constexpr auto const font_file{ FONTS_DIR FONT_FILE_NAME };

} // namespace alf
