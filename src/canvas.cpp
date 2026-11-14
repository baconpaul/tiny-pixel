#include "to_pixels/canvas.h"

#include <algorithm>
#include <cmath>

#include "blend.h"
#include "to_pixels/font.h"

namespace tpx
{

Canvas::Canvas(Surface s) : surf(s) { cur.clip = s.bounds(); }

void Canvas::save() { stack.push_back(cur); }

void Canvas::restore()
{
    if (stack.empty())
        return;
    cur = stack.back();
    stack.pop_back();
}

void Canvas::translate(int dx, int dy)
{
    cur.tx += dx;
    cur.ty += dy;
}

void Canvas::clipTo(const Rect &r) { cur.clip = cur.clip.intersect(r.translated(cur.tx, cur.ty)); }

Rect Canvas::clip() const
{
    if (cur.clip.empty())
        return {};
    return cur.clip.translated(-cur.tx, -cur.ty);
}

void Canvas::fillRect(const Rect &r, Color c)
{
    const Rect d = r.translated(cur.tx, cur.ty).intersect(cur.clip);
    if (d.empty() || c.a == 0)
        return;
    if (c.a == 255)
    {
        const uint32_t v = c.xrgb();
        for (int y = d.y; y < d.bottom(); ++y)
        {
            auto *p = surf.row(y);
            std::fill(p + d.x, p + d.right(), v);
        }
        return;
    }
    for (int y = d.y; y < d.bottom(); ++y)
    {
        auto *p = surf.row(y);
        for (int x = d.x; x < d.right(); ++x)
            p[x] = detail::blend(p[x], c, c.a);
    }
}

} // namespace tpx

namespace tpx
{

void Canvas::drawText(Font &f, float x, float baselineY, std::string_view utf8, Color c)
{
    if (c.a == 0 || cur.clip.empty())
        return;
    const int by = int(std::lround(baselineY)) + cur.ty;
    float pen = x + float(cur.tx);
    uint32_t prev = 0;
    for (size_t i = 0; i < utf8.size();)
    {
        const uint32_t cp = decodeUtf8(utf8, i);
        if (prev)
            pen += f.kern(prev, cp);
        prev = cp;
        const auto &g = f.glyph(cp);
        const int gx = int(std::lround(pen)) + g.xoff, gy = by + g.yoff;
        pen += g.advance;
        const Rect d = Rect{gx, gy, g.w, g.h}.intersect(cur.clip);
        if (d.empty())
            continue;
        for (int y = d.y; y < d.bottom(); ++y)
        {
            auto *row = surf.row(y);
            const uint8_t *src = g.coverage.data() + size_t(y - gy) * size_t(g.w);
            for (int px = d.x; px < d.right(); ++px)
            {
                const uint32_t cov = src[px - gx];
                if (!cov)
                    continue;
                row[px] = detail::blend(row[px], c, detail::alphaFor(c, cov));
            }
        }
    }
}

void Canvas::drawText(Font &f, const Rect &box, std::string_view utf8, Color c, Align align)
{
    save();
    clipTo(box);
    float x = float(box.x);
    if (align != Align::Left)
    {
        const float w = f.textWidth(utf8);
        x = align == Align::Centre ? float(box.x) + (float(box.w) - w) * 0.5f
                                   : float(box.right()) - w;
    }
    const float baseline = float(box.y) + (float(box.h) + float(f.capHeight())) * 0.5f;
    drawText(f, x, baseline, utf8, c);
    restore();
}

} // namespace tpx
