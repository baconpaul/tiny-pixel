#include "catch2.hpp"

#include <vector>

#include "tiny-pixel/canvas.h"

TEST_CASE("fillRect writes opaque colour inside and nothing outside", "[canvas]")
{
    tpx::Image img(8, 8);
    tpx::Canvas c(img.surface());
    c.fillRect({2, 3, 3, 2}, tpx::Color::rgb(0x112233));
    REQUIRE(img.at(2, 3) == 0xFF112233u);
    REQUIRE(img.at(4, 4) == 0xFF112233u);
    REQUIRE(img.at(5, 3) == 0xFF000000u);
    REQUIRE(img.at(2, 5) == 0xFF000000u);
}

TEST_CASE("fillRect respects stride padding", "[canvas]")
{
    std::vector<uint32_t> buf(6 * 4, 0xDEADBEEF); // 4 wide, stride 6
    tpx::Surface s{buf.data(), 4, 4, 6 * 4};
    tpx::Canvas c(s);
    c.fillRect({-5, -5, 100, 100}, tpx::Color::rgb(0xFFFFFF));
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 6; ++x)
            REQUIRE(buf[y * 6 + x] == (x < 4 ? 0xFFFFFFFFu : 0xDEADBEEFu));
}

TEST_CASE("fillRect blends half alpha", "[canvas]")
{
    tpx::Image img(1, 1, 0xFF000000);
    tpx::Canvas c(img.surface());
    c.fillRect({0, 0, 1, 1}, tpx::Color::rgb(0xFF0000, 128));
    auto r = (img.at(0, 0) >> 16) & 0xFF;
    REQUIRE(r >= 127);
    REQUIRE(r <= 129);
    REQUIRE((img.at(0, 0) >> 24) == 0xFFu);
}

TEST_CASE("Rect intersect and unite", "[canvas]")
{
    REQUIRE((tpx::Rect{0, 0, 10, 10}.intersect({5, 5, 10, 10}) == tpx::Rect{5, 5, 5, 5}));
    REQUIRE(tpx::Rect{0, 0, 2, 2}.intersect({5, 5, 1, 1}).empty());
    REQUIRE((tpx::Rect{}.unite({1, 1, 2, 2}) == tpx::Rect{1, 1, 2, 2}));
}

static int red(const tpx::Image &i, int x, int y) { return (i.at(x, y) >> 16) & 0xFF; }

TEST_CASE("fillCircle is solid inside, empty outside, partial on the edge", "[canvas]")
{
    tpx::Image img(20, 20);
    tpx::Canvas c(img.surface());
    c.fillCircle({10, 10}, 5, tpx::Color::rgb(0xFFFFFF));
    REQUIRE(img.at(9, 9) == 0xFFFFFFFFu);
    REQUIRE(img.at(0, 0) == 0xFF000000u);
    REQUIRE(red(img, 16, 10) == 0);
    REQUIRE(red(img, 13, 13) > 60); // centre 13.5,13.5 sits ~0.05 inside
    REQUIRE(red(img, 13, 13) < 220);
}

TEST_CASE("clip and translate stack", "[canvas]")
{
    tpx::Image img(10, 10);
    tpx::Canvas c(img.surface());
    c.save();
    c.translate(2, 2);
    c.clipTo({0, 0, 3, 3});
    REQUIRE((c.clip() == tpx::Rect{0, 0, 3, 3}));
    c.fillRect({-10, -10, 100, 100}, tpx::Color::rgb(0xFFFFFF));
    c.restore();
    REQUIRE(img.at(2, 2) == 0xFFFFFFFFu);
    REQUIRE(img.at(4, 4) == 0xFFFFFFFFu);
    REQUIRE(img.at(5, 5) == 0xFF000000u);
    REQUIRE(img.at(1, 1) == 0xFF000000u);
    REQUIRE((c.clip() == tpx::Rect{0, 0, 10, 10}));
}

TEST_CASE("strokeArc covers its sweep and not the gap", "[canvas]")
{
    tpx::Image img(40, 40);
    tpx::Canvas c(img.surface());
    const float pi = 3.14159265f;
    c.strokeArc({20, 20}, 12, -0.75f * pi, 0.75f * pi, 3, tpx::Color::rgb(0xFFFFFF));
    REQUIRE(red(img, 19, 7) > 200);  // 12 o'clock, on the arc
    REQUIRE(red(img, 32, 19) > 200); // 3 o'clock
    REQUIRE(red(img, 19, 32) == 0);  // 6 o'clock is in the gap
}

TEST_CASE("strokeRoundedRect stays inside its rect", "[canvas]")
{
    tpx::Image img(20, 20);
    tpx::Canvas c(img.surface());
    c.strokeRoundedRect({2, 2, 16, 16}, 4, 2, tpx::Color::rgb(0xFFFFFF));
    REQUIRE(red(img, 1, 10) == 0);
    REQUIRE(red(img, 2, 10) > 200);
    REQUIRE(red(img, 10, 10) == 0);
}

TEST_CASE("gradient goes from top colour to bottom colour", "[canvas]")
{
    tpx::Image img(4, 20);
    tpx::Canvas c(img.surface());
    c.fillRoundedRect({0, 0, 4, 20}, 0, tpx::Color::rgb(0x000000), tpx::Color::rgb(0xFF0000));
    REQUIRE(red(img, 1, 0) < 20);
    REQUIRE(red(img, 1, 19) > 235);
}

TEST_CASE("degenerate and off-surface shapes write nothing out of bounds", "[canvas]")
{
    std::vector<uint32_t> buf(12 * 10, 0xDEADBEEF); // 10x10 with stride 12
    tpx::Surface s{buf.data(), 10, 10, 12 * 4};
    tpx::Canvas c(s);
    auto w = tpx::Color::rgb(0xFFFFFF);
    c.fillCircle({-50, -50}, 10, w);
    c.fillCircle({5, 5}, -3, w);
    c.fillRoundedRect({-3, -3, 30, 30}, 400, w);
    c.strokeRoundedRect({1, 1, 0, 5}, 2, 2, w);
    c.strokeArc({5, 5}, 20, 0, 7, 4, w);
    c.line({-100, 5}, {100, 5}, 3, w);
    for (int y = 0; y < 10; ++y)
        for (int x = 10; x < 12; ++x)
            REQUIRE(buf[y * 12 + x] == 0xDEADBEEFu);
}

TEST_CASE("a zero sweep arc paints only its cap, not the opposite side", "[canvas]")
{
    tpx::Image img(40, 40);
    tpx::Canvas c(img.surface());
    const float pi = 3.14159265f;
    c.strokeArc({20, 20}, 12, -0.75f * pi, -0.75f * pi, 4, tpx::Color::rgb(0xFFFFFF));
    REQUIRE(red(img, 11, 28) > 100); // the cap at 7:30
    for (int y = 0; y < 20; ++y)
        for (int x = 20; x < 40; ++x)
            REQUIRE(red(img, x, y) == 0);
}
