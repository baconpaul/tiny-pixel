#ifndef TINY_PIXEL_GEOMETRY_H
#define TINY_PIXEL_GEOMETRY_H

#include <algorithm>

namespace tpx
{

struct Rect
{
    int x{0}, y{0}, w{0}, h{0};

    bool empty() const { return w <= 0 || h <= 0; }
    int right() const { return x + w; }
    int bottom() const { return y + h; }
    bool contains(int px, int py) const
    {
        return px >= x && py >= y && px < right() && py < bottom();
    }
    bool intersects(const Rect &o) const { return !intersect(o).empty(); }

    Rect intersect(const Rect &o) const
    {
        const int x0 = std::max(x, o.x), y0 = std::max(y, o.y);
        const int x1 = std::min(right(), o.right()), y1 = std::min(bottom(), o.bottom());
        if (x1 <= x0 || y1 <= y0)
            return {};
        return {x0, y0, x1 - x0, y1 - y0};
    }

    Rect unite(const Rect &o) const
    {
        if (empty())
            return o.empty() ? Rect{} : o;
        if (o.empty())
            return *this;
        const int x0 = std::min(x, o.x), y0 = std::min(y, o.y);
        const int x1 = std::max(right(), o.right()), y1 = std::max(bottom(), o.bottom());
        return {x0, y0, x1 - x0, y1 - y0};
    }

    Rect translated(int dx, int dy) const { return {x + dx, y + dy, w, h}; }
    Rect reduced(int by) const { return {x + by, y + by, w - 2 * by, h - 2 * by}; }

    bool operator==(const Rect &) const = default;
};

struct PointF
{
    float x{0}, y{0};
};

struct RectF
{
    float x{0}, y{0}, w{0}, h{0};

    static RectF from(const Rect &r) { return {float(r.x), float(r.y), float(r.w), float(r.h)}; }
};

} // namespace tpx

#endif
