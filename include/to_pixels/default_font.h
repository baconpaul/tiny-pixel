#ifndef TO_PIXELS_DEFAULT_FONT_H
#define TO_PIXELS_DEFAULT_FONT_H

#include <span>

// link to_pixels::default_font for these
namespace tpx::fonts
{
std::span<const unsigned char> jetbrainsMonoRegular();
std::span<const unsigned char> jetbrainsMonoBold();
} // namespace tpx::fonts

#endif
