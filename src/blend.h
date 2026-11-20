#ifndef TINY_PIXEL_SRC_BLEND_H
#define TINY_PIXEL_SRC_BLEND_H

#include <cstdint>

#include "tiny-pixel/color.h"

namespace tpx::detail
{

// alpha is 0..255, already folded with coverage
inline uint32_t blend(uint32_t dst, Color c, uint32_t alpha)
{
    if (alpha >= 255)
        return c.xrgb();
    const uint32_t inv = 255 - alpha;
    auto ch = [&](uint32_t d, uint32_t s) {
        const uint32_t v = s * alpha + d * inv + 128;
        return (v + (v >> 8)) >> 8;
    };
    const uint32_t r = ch((dst >> 16) & 0xFF, c.r);
    const uint32_t g = ch((dst >> 8) & 0xFF, c.g);
    const uint32_t b = ch(dst & 0xFF, c.b);
    return 0xFF000000u | (r << 16) | (g << 8) | b;
}

// colour alpha folded with 0..255 coverage
inline uint32_t alphaFor(Color c, uint32_t coverage)
{
    const uint32_t v = c.a * coverage + 128;
    return (v + (v >> 8)) >> 8;
}

} // namespace tpx::detail

#endif
