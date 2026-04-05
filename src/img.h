#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bg
{

struct GetImageResult {
    std::vector<std::uint8_t> bytes{};
    std::uint32_t width{};
    std::uint32_t height{};
};

/// Get raw image bytes from an image file in the form of RGBRGBRBG.
GetImageResult loadImage(std::string const& imagePath, bool flippedVertically = false);

} // namespace bg
