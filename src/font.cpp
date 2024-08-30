#include "src/font.h"
#include "src/common.h"

namespace Alf
{

TTF_Font &get_font()
{
    static TTF_Font *const font = []() {
        auto result = TTF_OpenFont(font_file, 18);

        if (result == nullptr)
        {
            fatal_error("Font was nullptr.");
        }

        return result;
    }();
    return *font;
}

} // namespace Alf
