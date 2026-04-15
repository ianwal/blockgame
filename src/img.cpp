#include "src/img.h"

#include "src/lib/CImg.h"

#include <string>
#include <utility>
#include <vector>

namespace bg
{

// Note: This function is in its own file to reduce compile time when modifying other files because CImg is very large.

GetImageResult loadImage(std::string const& imagePath, bool flippedVertically)
{
    cimg_library::CImg<unsigned char> img(std::string{ imagePath }.c_str());
    if (flippedVertically) {
        // Flip the image vertically because OpenGL texture origin is bottom-left first but most image formats store in
        // top-left first.
        img.mirror('y');
    }

    int const width    = img.width();
    int const height   = img.height();
    int const channels = img.spectrum(); // Actual number of channels in the file

    // Fix the color channel interleaving. CImg stores as RRRGGGBBBAAA but OpenGL needs RGBARGBARBGA.
    std::vector<unsigned char> interleaved(width * height * 4);
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++) {
            interleaved[(y * width + x) * 4 + 0] = img(x, y, 0, 0); // R
            interleaved[(y * width + x) * 4 + 1] = img(x, y, 0, 1); // G
            interleaved[(y * width + x) * 4 + 2] = img(x, y, 0, 2); // B
            // Premultiply alpha so bilinear filtering doesn't blend in RGB values from transparent pixels,
            // which causes a colored fringe at transparent edges. Without this, white can be seen on the block break
            // progress texture.
            float a                              = (channels >= 4 ? img(x, y, 0, 3) : 255) / 255.0f;
            interleaved[(y * width + x) * 4 + 3] = static_cast<unsigned char>(a * 255);
            interleaved[(y * width + x) * 4 + 0] *= a;
            interleaved[(y * width + x) * 4 + 1] *= a;
            interleaved[(y * width + x) * 4 + 2] *= a;
        }
    return GetImageResult{ std::move(interleaved), static_cast<std::uint32_t>(width),
                           static_cast<std::uint32_t>(height) };
}

} // namespace bg
