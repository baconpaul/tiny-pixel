#ifndef TO_PIXELS_FONT_H
#define TO_PIXELS_FONT_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace tpx
{

// returns U+FFFD and advances one byte on invalid input
uint32_t decodeUtf8(std::string_view s, size_t &i);

// a ttf at one pixel size; the caller keeps the ttf bytes alive
class Font
{
  public:
    Font(const unsigned char *ttf, size_t size, float pixelHeight);
    ~Font();
    Font(Font &&) noexcept;
    Font &operator=(Font &&) noexcept;
    Font(const Font &) = delete;
    Font &operator=(const Font &) = delete;

    struct Glyph
    {
        int w{0}, h{0}, xoff{0}, yoff{0};
        float advance{0};
        std::vector<uint8_t> coverage;
    };

    bool valid() const;
    float pixelHeight() const;
    int ascent() const;
    // negative, below the baseline
    int descent() const;
    int lineHeight() const;
    int capHeight() const;

    float textWidth(std::string_view utf8);
    const Glyph &glyph(uint32_t codepoint);
    float kern(uint32_t a, uint32_t b);

  private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace tpx

#endif
