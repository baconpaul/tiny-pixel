#ifndef TINY_PIXEL_DEFAULT_FONT_H
#define TINY_PIXEL_DEFAULT_FONT_H

#include <span>

// link tiny-pixel::default-font for these
namespace tpx::fonts
{
std::span<const unsigned char> jetbrainsMonoRegular();
std::span<const unsigned char> jetbrainsMonoBold();
} // namespace tpx::fonts

#endif
