#include "to_pixels/font.h"

#include <cmath>
#include <unordered_map>

// static so another stb_truetype in the same binary cannot clash
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace tpx
{

uint32_t decodeUtf8(std::string_view s, size_t &i)
{
    constexpr uint32_t bad = 0xFFFD;
    const auto b0 = uint8_t(s[i]);
    if (b0 < 0x80)
    {
        ++i;
        return b0;
    }
    int n;
    uint32_t cp;
    if ((b0 & 0xE0) == 0xC0)
    {
        n = 1;
        cp = b0 & 0x1F;
    }
    else if ((b0 & 0xF0) == 0xE0)
    {
        n = 2;
        cp = b0 & 0x0F;
    }
    else if ((b0 & 0xF8) == 0xF0)
    {
        n = 3;
        cp = b0 & 0x07;
    }
    else
    {
        ++i;
        return bad;
    }
    if (i + size_t(n) >= s.size())
    {
        ++i;
        return bad;
    }
    for (int k = 1; k <= n; ++k)
    {
        const auto b = uint8_t(s[i + size_t(k)]);
        if ((b & 0xC0) != 0x80)
        {
            ++i;
            return bad;
        }
        cp = (cp << 6) | (b & 0x3F);
    }
    static constexpr uint32_t minFor[4] = {0, 0x80, 0x800, 0x10000};
    if (cp < minFor[n] || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
    {
        ++i;
        return bad;
    }
    i += size_t(n) + 1;
    return cp;
}

struct Font::Impl
{
    stbtt_fontinfo info{};
    bool ok{false};
    float px{0}, scale{0};
    int ascent{0}, descent{0}, lineGap{0}, capHeight{0};
    std::unordered_map<uint32_t, Glyph> cache;
};

Font::Font(const unsigned char *ttf, size_t size, float pixelHeight)
    : impl(std::make_unique<Impl>())
{
    impl->px = pixelHeight;
    if (!ttf || size < 12 || !(pixelHeight > 0.f))
        return;
    const int offset = stbtt_GetFontOffsetForIndex(ttf, 0);
    if (offset < 0 || !stbtt_InitFont(&impl->info, ttf, offset))
        return;
    impl->ok = true;
    impl->scale = stbtt_ScaleForPixelHeight(&impl->info, pixelHeight);
    int a, d, g;
    stbtt_GetFontVMetrics(&impl->info, &a, &d, &g);
    impl->ascent = int(std::lround(float(a) * impl->scale));
    impl->descent = int(std::lround(float(d) * impl->scale));
    impl->lineGap = int(std::lround(float(g) * impl->scale));
    int x0, y0, x1, y1;
    if (stbtt_GetCodepointBox(&impl->info, 'H', &x0, &y0, &x1, &y1))
        impl->capHeight = int(std::lround(float(y1) * impl->scale));
    else
        impl->capHeight = int(std::lround(float(impl->ascent) * 0.7f));
}

Font::~Font() = default;
Font::Font(Font &&) noexcept = default;
Font &Font::operator=(Font &&) noexcept = default;

bool Font::valid() const { return impl && impl->ok; }
float Font::pixelHeight() const { return impl ? impl->px : 0.f; }
int Font::ascent() const { return impl ? impl->ascent : 0; }
int Font::descent() const { return impl ? impl->descent : 0; }
int Font::lineHeight() const { return impl ? impl->ascent - impl->descent + impl->lineGap : 0; }
int Font::capHeight() const { return impl ? impl->capHeight : 0; }

const Font::Glyph &Font::glyph(uint32_t codepoint)
{
    auto it = impl->cache.find(codepoint);
    if (it != impl->cache.end())
        return it->second;
    Glyph g;
    if (impl->ok)
    {
        const int idx = stbtt_FindGlyphIndex(&impl->info, int(codepoint));
        int adv, lsb;
        stbtt_GetGlyphHMetrics(&impl->info, idx, &adv, &lsb);
        g.advance = float(adv) * impl->scale;
        if (idx != 0)
        {
            int x0, y0, x1, y1;
            stbtt_GetGlyphBitmapBox(&impl->info, idx, impl->scale, impl->scale, &x0, &y0, &x1, &y1);
            if (x1 > x0 && y1 > y0)
            {
                g.w = x1 - x0;
                g.h = y1 - y0;
                g.xoff = x0;
                g.yoff = y0;
                g.coverage.resize(size_t(g.w) * size_t(g.h));
                stbtt_MakeGlyphBitmap(&impl->info, g.coverage.data(), g.w, g.h, g.w, impl->scale,
                                      impl->scale, idx);
            }
        }
    }
    return impl->cache.emplace(codepoint, std::move(g)).first->second;
}

float Font::kern(uint32_t a, uint32_t b)
{
    if (!impl->ok)
        return 0.f;
    return float(stbtt_GetCodepointKernAdvance(&impl->info, int(a), int(b))) * impl->scale;
}

float Font::textWidth(std::string_view utf8)
{
    float w = 0.f;
    uint32_t prev = 0;
    for (size_t i = 0; i < utf8.size();)
    {
        const uint32_t cp = decodeUtf8(utf8, i);
        if (prev)
            w += kern(prev, cp);
        w += glyph(cp).advance;
        prev = cp;
    }
    return w;
}

} // namespace tpx
