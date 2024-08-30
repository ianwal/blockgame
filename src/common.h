#pragma once

namespace Alf
{

struct Dim2d
{
    int width{};
    int height{};
};

void fatal_error(const char *const msg);

#define ASSETS_DIR "assets/"

#define FONT_FILE_NAME "roboto-regular.ttf"

#define FONTS_DIR ASSETS_DIR "fonts/"

constexpr auto const font_file{FONTS_DIR FONT_FILE_NAME};

} // namespace Alf
