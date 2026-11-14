#ifndef TO_PIXELS_THEME_H
#define TO_PIXELS_THEME_H

#include "color.h"

namespace tpx
{

class Font;

// colour names follow sndstation's style so a device's colours copy straight in
struct Theme
{
    Color bg0{Color::rgb(0x14161a)};
    Color bg1{Color::rgb(0x1d2026)};
    Color bg2{Color::rgb(0x272b33)};
    Color line{Color::rgb(0x3a3f4a)};
    Color txt0{Color::rgb(0xeef0f4)};
    Color txt1{Color::rgb(0xaab0bc)};
    Color txt2{Color::rgb(0x6c7380)};
    Color accL{Color::rgb(0xff8a3d)};
    Color accR{Color::rgb(0x2ec4b6)};
    Color warn{Color::rgb(0xff4d4d)};

    float radius{6};
    float focusRingWidth{2};

    // not owned; widgets skip text when null
    Font *font{nullptr};
    Font *fontBold{nullptr};
    Font *fontSmall{nullptr};
    Font *fontValue{nullptr};

    float repeatDelay{0.35f}, repeatSlow{0.09f}, repeatFast{0.03f}, repeatFastAfter{1.0f};
};

} // namespace tpx

#endif
