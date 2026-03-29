#include "src/img.h"

#include "src/lib/CImg.h"

#include <string>
#include <utility>
#include <vector>

namespace Alf
{

// Note: This function is in its own file to reduce compile time when modifying other files because CImg is very large.

GetImageResult loadImage(std::string const& imagePath, bool flippedVertically)
{
    cimg_library::CImg<unsigned char> img(std::string{ imagePath }.c_str());
    if (flippedVertically) {
        img.mirror('y'); // Flip the image vertically because OpenGL handles them flipped or something.
    }

    int const width  = img.width();
    int const height = img.height();

    // Fix the color channel interleaving. CImg stores as RRRGGGBBB but OpenGL needs RGBRGBRBG.
    std::vector<unsigned char> interleaved(width * height * 3);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            interleaved[(y * width + x) * 3 + 0] = img(x, y, 0, 0); // R
            interleaved[(y * width + x) * 3 + 1] = img(x, y, 0, 1); // G
            interleaved[(y * width + x) * 3 + 2] = img(x, y, 0, 2); // B
        }
    return GetImageResult{ std::move(interleaved), static_cast<std::uint32_t>(width),
                           static_cast<std::uint32_t>(height) };
}

} // namespace Alf
