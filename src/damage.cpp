#include "to_pixels/damage.h"

namespace tpx
{

namespace
{
bool touches(const Rect &a, const Rect &b)
{
    return a.x <= b.right() && b.x <= a.right() && a.y <= b.bottom() && b.y <= a.bottom();
}
} // namespace

void DamageRegion::add(const Rect &r)
{
    if (r.empty())
        return;
    Rect m = r;
    bool merged = true;
    while (merged)
    {
        merged = false;
        for (size_t i = 0; i < rs.size(); ++i)
        {
            if (touches(rs[i], m))
            {
                m = m.unite(rs[i]);
                rs.erase(rs.begin() + std::ptrdiff_t(i));
                merged = true;
                break;
            }
        }
    }
    rs.push_back(m);
    if (rs.size() > maxRects)
    {
        const Rect b = bounds();
        rs.assign(1, b);
    }
}

void DamageRegion::addAll(const DamageRegion &o)
{
    for (const auto &r : o.rs)
        add(r);
}

Rect DamageRegion::bounds() const
{
    Rect b;
    for (const auto &r : rs)
        b = b.unite(r);
    return b;
}

} // namespace tpx
