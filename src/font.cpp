#include "src/font.h"
#include "src/common.h"

#include <memory>

namespace Alf
{

namespace
{

// smart pointer wrapper around TTF_Font
struct TTF_Font_Deleter {
    void operator()(TTF_Font* ptr) const
    {
        TTF_CloseFont(ptr);
    }
};
using TTF_Font_Ptr = std::unique_ptr<TTF_Font, TTF_Font_Deleter>;

TTF_Font_Ptr main_font{ nullptr };

} // namespace

// CALL ONCE AT START OF PROGRAM.
bool global_font_init()
{
    assert(main_font == nullptr);
    main_font = TTF_Font_Ptr{ TTF_OpenFont(font_file, 12) };
    return (main_font == nullptr);
}

// CALL ONCE AT END OF PROGRAM.
void destroy_global_font()
{
    main_font.reset();
}

// call global_font_init() before this
TTF_Font& get_font()
{
    return *(main_font.get());
}

} // namespace Alf
