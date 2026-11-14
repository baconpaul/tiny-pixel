#include "catch2.hpp"

#include <string>

#include "to_pixels/canvas.h"
#include "to_pixels/default_font.h"
#include "to_pixels/font.h"

static tpx::Font mono(float px)
{
    auto d = tpx::fonts::jetbrainsMonoRegular();
    return tpx::Font(d.data(), d.size(), px);
}

TEST_CASE("font loads and measures", "[font]")
{
    auto f = mono(20);
    REQUIRE(f.valid());
    REQUIRE(f.ascent() > 0);
    REQUIRE(f.descent() < 0);
    REQUIRE(f.capHeight() > 8);
    REQUIRE(f.capHeight() < 20);
    // monospaced: width is proportional to length
    REQUIRE(f.textWidth("abcd") == Approx(2 * f.textWidth("ab")).margin(1.0));
    REQUIRE(f.textWidth("") == 0);
}

TEST_CASE("glyphs are cached", "[font]")
{
    auto f = mono(16);
    const auto *a = &f.glyph('A');
    REQUIRE(a == &f.glyph('A'));
    REQUIRE(a->w > 0);
}

TEST_CASE("invalid utf8 and missing glyphs do not crash", "[font]")
{
    auto f = mono(16);
    size_t i = 0;
    std::string bad("\xff"
                    "a",
                    2);
    REQUIRE(tpx::decodeUtf8(bad, i) == 0xFFFDu);
    REQUIRE(i == 1);
    tpx::Image img(64, 32);
    tpx::Canvas c(img.surface());
    c.drawText(f, 2, 20, "\xff\xfe\xf0\x9f\x98\x80 ok", tpx::Color::rgb(0xFFFFFF));
    REQUIRE(f.textWidth("\xf0\x9f\x98\x80") > 0); // missing emoji still advances
}

TEST_CASE("boxed text is clipped to its box", "[font]")
{
    auto f = mono(16);
    tpx::Image img(100, 30);
    tpx::Canvas c(img.surface());
    c.drawText(f, tpx::Rect{10, 5, 20, 20}, "WWWWWWWWWWWW", tpx::Color::rgb(0xFFFFFF),
               tpx::Align::Left);
    for (int y = 0; y < 30; ++y)
        REQUIRE(img.at(40, y) == 0xFF000000u);
    bool any = false;
    for (int y = 5; y < 25; ++y)
        any |= img.at(12, y) != 0xFF000000u;
    REQUIRE(any);
}
