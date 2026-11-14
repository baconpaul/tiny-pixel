#include "catch2.hpp"

#include <cmath>

#include "png.h"
#include "to_pixels/canvas.h"

// visual review only: ./build/to_pixels_tests "[.shots]" then look at build/shots
namespace
{
const auto bg0 = tpx::Color::rgb(0x14161a);
const auto bg2 = tpx::Color::rgb(0x272b33);
const auto line = tpx::Color::rgb(0x3a3f4a);
const auto txt0 = tpx::Color::rgb(0xeef0f4);
const auto accL = tpx::Color::rgb(0xff8a3d);
const auto accR = tpx::Color::rgb(0x2ec4b6);
constexpr float pi = 3.14159265f;
} // namespace

TEST_CASE("canvas primitives", "[.shots]")
{
    tpx::Image img(480, 320);
    tpx::Canvas c(img.surface());
    c.fillRect({0, 0, 480, 320}, bg0);

    c.fillRoundedRect({10, 10, 120, 60}, 8, bg2);
    c.fillRoundedRect({140, 10, 120, 60}, 12, tpx::Color::rgb(0x3a3f4a), tpx::Color::rgb(0x1d2026));
    c.strokeRoundedRect({270, 10, 120, 60}, 8, 2, accL);
    c.strokeRoundedRect({400, 10, 70, 60}, 30, 1.5f, accR);

    for (int i = 0; i < 6; ++i)
    {
        const float r = 3.f + float(i) * 5.f;
        c.fillCircle({20.f + float(i) * 45.f + 0.3f * float(i), 110}, r, txt0);
    }
    c.strokeCircle({330, 110}, 25, 3, accR);
    c.strokeCircle({400, 110}, 25, 1, txt0);

    for (int i = 0; i < 5; ++i)
    {
        const float cx = 50.f + float(i) * 90.f, cy = 200.f;
        const float v = float(i) / 4.f;
        const float a0 = -0.75f * pi, a1 = a0 + v * 1.5f * pi;
        c.strokeArc({cx, cy}, 30, a0, 0.75f * pi, 6, line);
        c.strokeArc({cx, cy}, 30, a0, a1, 6, accL);
        c.fillCircle({cx, cy}, 20, bg2);
        c.line({cx + std::sin(a1) * 5, cy - std::cos(a1) * 5},
               {cx + std::sin(a1) * 17, cy - std::cos(a1) * 17}, 3, txt0);
    }

    for (int i = 0; i < 8; ++i)
        c.line({10.f + float(i) * 58.f, 300}, {50.f + float(i) * 58.f, 300.f - float(i) * 6.f},
               1.f + float(i) * 0.5f, txt0);

    REQUIRE(tpx::test::writePng(img, "build/shots/canvas.png"));
}

#include "to_pixels/default_font.h"
#include "to_pixels/font.h"

TEST_CASE("text at sizes", "[.shots]")
{
    auto reg = tpx::fonts::jetbrainsMonoRegular();
    auto bold = tpx::fonts::jetbrainsMonoBold();
    tpx::Image img(480, 260);
    tpx::Canvas c(img.surface());
    c.fillRect({0, 0, 480, 260}, bg0);
    float y = 10;
    for (float px : {12.f, 16.f, 24.f, 40.f})
    {
        tpx::Font r(reg.data(), reg.size(), px), b(bold.data(), bold.size(), px);
        y += float(r.ascent()) + 4;
        c.drawText(r, 10, y, "Cutoff 1.2 kHz", txt0);
        c.drawText(b, 250, y, "Preset Ag", accL);
        y += float(-r.descent()) + 4;
    }
    tpx::Font r(reg.data(), reg.size(), 16);
    c.fillRect({10, 200, 140, 40}, bg2);
    c.drawText(r, {10, 200, 140, 40}, "Left", txt0, tpx::Align::Left);
    c.fillRect({160, 200, 140, 40}, bg2);
    c.drawText(r, {160, 200, 140, 40}, "Centre", txt0, tpx::Align::Centre);
    c.fillRect({310, 200, 140, 40}, bg2);
    c.drawText(r, {310, 200, 140, 40}, "Right", txt0, tpx::Align::Right);
    REQUIRE(tpx::test::writePng(img, "build/shots/text.png"));
}
