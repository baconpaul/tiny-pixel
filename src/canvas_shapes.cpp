#include "tiny-pixel/canvas.h"

#include <algorithm>
#include <cmath>

#include "blend.h"

namespace tpx
{

namespace
{

constexpr float twoPi = 6.28318531f;

// pixels [x0, x1) of a row are all fully covered (solid) or all empty
struct RowSpan
{
    int x0{0}, x1{0};
    bool solid{false};
};

// pixel indices whose centres lie in [a, b]
RowSpan centresIn(float a, float b, bool solid)
{
    if (!(a <= b))
        return {};
    const int i0 = int(std::ceil(a - 0.5f));
    const int i1 = std::max(i0, int(std::floor(b - 0.5f)) + 1);
    return {i0, i1, solid};
}

// coverage is clamp(0.5 - d, 0, 1) at each pixel centre
template <class Sdf, class Span, class RowColour>
void raster(const Surface &s, const Rect &clip, float bx0, float by0, float bx1, float by1, Sdf sdf,
            Span span, RowColour colour)
{
    if (!(bx0 < bx1) || !(by0 < by1) || clip.empty())
        return;
    const int x0 = int(std::max(float(clip.x), std::floor(bx0 - 1)));
    const int x1 = int(std::min(float(clip.right()), std::ceil(bx1 + 1)));
    const int y0 = int(std::max(float(clip.y), std::floor(by0 - 1)));
    const int y1 = int(std::min(float(clip.bottom()), std::ceil(by1 + 1)));
    if (x1 <= x0 || y1 <= y0)
        return;

    for (int y = y0; y < y1; ++y)
    {
        const float py = float(y) + 0.5f;
        const Color c = colour(py);
        if (c.a == 0)
            continue;
        const RowSpan sp = span(py);
        const int s0 = std::clamp(sp.x0, x0, x1);
        const int s1 = std::clamp(sp.x1, s0, x1);
        auto *row = s.row(y);

        auto edge = [&](int from, int to) {
            for (int x = from; x < to; ++x)
            {
                const float cov = 0.5f - sdf(float(x) + 0.5f, py);
                if (cov <= 0.f)
                    continue;
                const uint32_t c255 = cov >= 1.f ? 255u : uint32_t(cov * 255.f + 0.5f);
                const uint32_t a = detail::alphaFor(c, c255);
                if (a)
                    row[x] = detail::blend(row[x], c, a);
            }
        };

        edge(x0, s0);
        if (sp.solid && s1 > s0)
        {
            if (c.a == 255)
                std::fill(row + s0, row + s1, c.xrgb());
            else
                for (int x = s0; x < s1; ++x)
                    row[x] = detail::blend(row[x], c, c.a);
        }
        edge(s1, x1);
    }
}

struct RoundRect
{
    float x, y, w, h, r;

    float sdf(float px, float py) const
    {
        const float hx = w * 0.5f, hy = h * 0.5f;
        const float qx = std::fabs(px - (x + hx)) - (hx - r);
        const float qy = std::fabs(py - (y + hy)) - (hy - r);
        const float ox = std::max(qx, 0.f), oy = std::max(qy, 0.f);
        return std::sqrt(ox * ox + oy * oy) + std::min(std::max(qx, qy), 0.f) - r;
    }

    // pixels certain to have d <= -0.5
    RowSpan interior(float py, bool solid) const
    {
        if (py >= y + r + 0.5f && py <= y + h - r - 0.5f)
            return centresIn(x + 0.5f, x + w - 0.5f, solid);
        if (py >= y + 0.5f && py <= y + h - 0.5f)
            return centresIn(x + r + 0.5f, x + w - r - 0.5f, solid);
        return {};
    }
};

RoundRect makeRoundRect(const RectF &r, float radius, int tx, int ty)
{
    const float maxR = std::min(r.w, r.h) * 0.5f;
    const float rad = std::isfinite(radius) ? std::clamp(radius, 0.f, maxR) : 0.f;
    return {r.x + float(tx), r.y + float(ty), r.w, r.h, rad};
}

// pixels certain to lie inside a circle by at least half a pixel
RowSpan discInterior(float py, float cx, float cy, float r, bool solid)
{
    const float ri = r - 0.5f;
    const float dy = std::fabs(py - cy);
    if (!(ri > 0.f) || dy >= ri)
        return {};
    const float half = std::sqrt(ri * ri - dy * dy);
    return centresIn(cx - half, cx + half, solid);
}

auto flat(Color c)
{
    return [c](float) { return c; };
}

} // namespace

void Canvas::fillRoundedRect(const RectF &r, float radius, Color c)
{
    fillRoundedRect(r, radius, c, c);
}

void Canvas::fillRoundedRect(const RectF &r, float radius, Color top, Color bottom)
{
    if (!(r.w > 0.f) || !(r.h > 0.f))
        return;
    const RoundRect rr = makeRoundRect(r, radius, cur.tx, cur.ty);
    raster(
        surf, cur.clip, rr.x, rr.y, rr.x + rr.w, rr.y + rr.h,
        [&](float px, float py) { return rr.sdf(px, py); },
        [&](float py) { return rr.interior(py, true); },
        [&](float py) {
            return top == bottom ? top : Color::mix(top, bottom, (py - rr.y) / rr.h);
        });
}

void Canvas::strokeRoundedRect(const RectF &r, float radius, float width, Color c)
{
    if (!(r.w > 0.f) || !(r.h > 0.f) || !(width > 0.f))
        return;
    const RoundRect outer = makeRoundRect(r, radius, cur.tx, cur.ty);
    const float w = std::min(width, std::min(r.w, r.h) * 0.5f);
    const RoundRect inner{outer.x + w, outer.y + w, outer.w - 2 * w, outer.h - 2 * w,
                          std::max(outer.r - w, 0.f)};
    const bool hollow = inner.w > 0.f && inner.h > 0.f;
    raster(
        surf, cur.clip, outer.x, outer.y, outer.x + outer.w, outer.y + outer.h,
        [&](float px, float py) {
            const float d = outer.sdf(px, py);
            return hollow ? std::max(d, -inner.sdf(px, py)) : d;
        },
        [&](float py) { return hollow ? inner.interior(py, false) : RowSpan{}; }, flat(c));
}

void Canvas::fillCircle(PointF centre, float r, Color c)
{
    if (!(r > 0.f))
        return;
    const float cx = centre.x + float(cur.tx), cy = centre.y + float(cur.ty);
    raster(
        surf, cur.clip, cx - r, cy - r, cx + r, cy + r,
        [&](float px, float py) { return std::hypot(px - cx, py - cy) - r; },
        [&](float py) { return discInterior(py, cx, cy, r, true); }, flat(c));
}

void Canvas::strokeCircle(PointF centre, float r, float width, Color c)
{
    if (!(r > 0.f) || !(width > 0.f))
        return;
    const float cx = centre.x + float(cur.tx), cy = centre.y + float(cur.ty);
    const float hw = width * 0.5f, ro = r + hw;
    raster(
        surf, cur.clip, cx - ro, cy - ro, cx + ro, cy + ro,
        [&](float px, float py) { return std::fabs(std::hypot(px - cx, py - cy) - r) - hw; },
        [&](float py) { return discInterior(py, cx, cy, r - hw, false); }, flat(c));
}

void Canvas::strokeArc(PointF centre, float r, float startRad, float endRad, float width, Color c)
{
    const float sweep = endRad - startRad;
    if (!(r > 0.f) || !(width > 0.f) || !(sweep >= 0.f))
        return;
    if (sweep >= twoPi)
    {
        strokeCircle(centre, r, width, c);
        return;
    }
    const float cx = centre.x + float(cur.tx), cy = centre.y + float(cur.ty);
    const float hw = width * 0.5f, ro = r + hw;
    const float ax = std::sin(startRad), ay = -std::cos(startRad);
    const float bx = std::sin(endRad), by = -std::cos(endRad);
    const bool small = sweep <= twoPi * 0.5f;
    // a near-zero sweep makes the wedge test accept the whole line, opposite ray included
    const bool narrow = sweep < twoPi * 0.25f;
    const float mx = ax + bx, my = ay + by;
    auto cross = [](float ux, float uy, float vx, float vy) { return ux * vy - uy * vx; };

    raster(
        surf, cur.clip, cx - ro, cy - ro, cx + ro, cy + ro,
        [&](float px, float py) {
            const float x = px - cx, y = py - cy;
            const bool inside = small ? (cross(ax, ay, x, y) >= 0.f && cross(x, y, bx, by) >= 0.f &&
                                         (!narrow || x * mx + y * my > 0.f))
                                      : !(cross(bx, by, x, y) > 0.f && cross(x, y, ax, ay) > 0.f);
            if (inside)
                return std::fabs(std::hypot(x, y) - r) - hw;
            const float da = std::hypot(x - r * ax, y - r * ay);
            const float db = std::hypot(x - r * bx, y - r * by);
            return std::min(da, db) - hw;
        },
        [&](float py) { return discInterior(py, cx, cy, r - hw, false); }, flat(c));
}

void Canvas::line(PointF a, PointF b, float width, Color c)
{
    if (!(width > 0.f))
        return;
    const float ax = a.x + float(cur.tx), ay = a.y + float(cur.ty);
    const float bx = b.x + float(cur.tx), by = b.y + float(cur.ty);
    const float hw = width * 0.5f;
    const float dx = bx - ax, dy = by - ay;
    const float len2 = dx * dx + dy * dy;
    raster(
        surf, cur.clip, std::min(ax, bx) - hw, std::min(ay, by) - hw, std::max(ax, bx) + hw,
        std::max(ay, by) + hw,
        [&](float px, float py) {
            const float qx = px - ax, qy = py - ay;
            const float t = len2 > 0.f ? std::clamp((qx * dx + qy * dy) / len2, 0.f, 1.f) : 0.f;
            return std::hypot(qx - t * dx, qy - t * dy) - hw;
        },
        [](float) { return RowSpan{}; }, flat(c));
}

} // namespace tpx
