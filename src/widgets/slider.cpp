#include "tiny-pixel/widgets/slider.h"

#include <algorithm>

#include "tiny-pixel/canvas.h"
#include "tiny-pixel/font.h"
#include "tiny-pixel/root.h"

namespace tpx
{

namespace
{
constexpr float pad = 12.f, trackW = 6.f, thumbR = 7.f;
} // namespace

void Slider::setOrientation(Orientation o)
{
    orient = o;
    repaint();
}

void Slider::paint(Canvas &c)
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
    const float v = value();

    if (orient == Orientation::Horizontal)
    {
        const int row = std::max(lrow, vrow), top = 6;
        if (lf)
            c.drawText(*lf, Rect{int(pad), top, w - 2 * int(pad), row}, label(), t.txt1,
                       Align::Left);
        if (vf)
            c.drawText(*vf, Rect{int(pad), top, w - 2 * int(pad), row}, valueText(), t.txt0,
                       Align::Right);
        const float x0 = pad, x1 = float(w) - pad;
        const float cy = float(row + top) + (float(h - row - top)) * 0.5f;
        const float tx = x0 + v * (x1 - x0);
        c.fillRoundedRect({x0, cy - trackW * 0.5f, x1 - x0, trackW}, trackW * 0.5f, t.bg2);
        float f0 = bipolar() ? (x0 + x1) * 0.5f : x0, f1 = tx;
        if (f1 < f0)
            std::swap(f0, f1);
        if (f1 - f0 > 0.5f)
            c.fillRoundedRect({f0, cy - trackW * 0.5f, f1 - f0, trackW}, trackW * 0.5f, t.accL);
        c.fillCircle({tx, cy}, thumbR, t.txt0);
        return;
    }

    const int textH = lrow + vrow + 4;
    if (lf)
        c.drawText(*lf, Rect{0, h - textH, w, lrow}, label(), t.txt1, Align::Centre);
    if (vf)
        c.drawText(*vf, Rect{0, h - textH + lrow, w, vrow}, valueText(), t.txt0, Align::Centre);
    const float y0 = pad, y1 = float(h - textH) - pad;
    if (y1 <= y0)
        return;
    const float cx = float(w) * 0.5f;
    const float ty = y1 - v * (y1 - y0);
    c.fillRoundedRect({cx - trackW * 0.5f, y0, trackW, y1 - y0}, trackW * 0.5f, t.bg2);
    float f0 = bipolar() ? (y0 + y1) * 0.5f : y1, f1 = ty;
    if (f1 < f0)
        std::swap(f0, f1);
    if (f1 - f0 > 0.5f)
        c.fillRoundedRect({cx - trackW * 0.5f, f0, trackW, f1 - f0}, trackW * 0.5f, t.accL);
    c.fillCircle({cx, ty}, thumbR, t.txt0);
}

} // namespace tpx
