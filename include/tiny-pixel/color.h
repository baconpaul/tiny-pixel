#ifndef TINY_PIXEL_COLOR_H
#define TINY_PIXEL_COLOR_H

#include <algorithm>
#include <cstdint>

namespace tpx
{

// straight (not premultiplied) alpha
struct Color
{
    uint8_t r{0}, g{0}, b{0}, a{255};

    static constexpr Color rgb(uint32_t rrggbb, uint8_t a = 255)
    {
        return {uint8_t((rrggbb >> 16) & 0xFF), uint8_t((rrggbb >> 8) & 0xFF),
                uint8_t(rrggbb & 0xFF), a};
    }

    constexpr Color withAlpha(uint8_t na) const { return {r, g, b, na}; }

    static Color mix(Color from, Color to, float t)
    {
        t = std::clamp(t, 0.f, 1.f);
        auto l = [t](uint8_t a, uint8_t b) { return uint8_t(a + (b - a) * t + 0.5f); };
        return {l(from.r, to.r), l(from.g, to.g), l(from.b, to.b), l(from.a, to.a)};
    }

    constexpr uint32_t xrgb() const
    {
        return 0xFF000000u | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
    }

    bool operator==(const Color &) const = default;
};

} // namespace tpx

#endif
