#include "to_pixels/widgets/knob.h"

#include <algorithm>
#include <cmath>

#include "to_pixels/canvas.h"
#include "to_pixels/font.h"
#include "to_pixels/root.h"

namespace tpx
{

namespace
{
constexpr float pi = 3.14159265f;
constexpr float arcStart = -0.75f * pi, arcSweep = 1.5f * pi;
} // namespace

void Knob::paint(Canvas &c)
{
    auto *r = root();
    if (!r)
        return;
    const Theme &t = r->theme();
    Font *lf = t.fontSmall ? t.fontSmall : t.font;
    Font *vf = t.fontValue ? t.fontValue : lf;
    const int lrow = lf ? lf->lineHeight() : 0;
    const int vrow = vf ? vf->lineHeight() : 0;
    const int w = bounds().w, h = bounds().h;
    const int knobH = h - lrow - vrow - 4;

    const float outer = float(std::min(w, knobH)) * 0.5f - 4.f;
    if (outer > 4.f)
    {
        const float cx = float(w) * 0.5f, cy = float(knobH) * 0.5f + 2.f;
        const float tw = std::max(3.f, outer * 0.14f);
        const float ra = outer - tw * 0.5f;
        const float angle = arcStart + value() * arcSweep;

        c.strokeArc({cx, cy}, ra, arcStart, arcStart + arcSweep, tw, t.line);
        float a0 = bipolar() ? 0.f : arcStart, a1 = angle;
        if (a1 < a0)
            std::swap(a0, a1);
        if (a1 - a0 > 0.001f)
            c.strokeArc({cx, cy}, ra, a0, a1, tw, t.accL);

        const float disc = outer * 0.68f;
        c.fillCircle({cx, cy}, disc, t.bg2);
        c.strokeCircle({cx, cy}, disc - 0.5f, 1.f, t.line);
        const float sx = std::sin(angle), sy = -std::cos(angle);
        c.line({cx + sx * outer * 0.25f, cy + sy * outer * 0.25f},
               {cx + sx * outer * 0.6f, cy + sy * outer * 0.6f}, std::max(2.f, outer * 0.08f),
               t.txt0);
    }

    if (lf)
        c.drawText(*lf, Rect{0, knobH + 2, w, lrow}, label(), t.txt1, Align::Centre);
    if (vf)
        c.drawText(*vf, Rect{0, knobH + 2 + lrow, w, vrow}, valueText(), t.txt0, Align::Centre);
}

} // namespace tpx
