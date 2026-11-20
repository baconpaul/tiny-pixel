#ifndef TINY_PIXEL_DAMAGE_H
#define TINY_PIXEL_DAMAGE_H

#include <cstddef>
#include <vector>

#include "geometry.h"

namespace tpx
{

// a few non-touching rects, collapsing to their bounds when there are too many
class DamageRegion
{
  public:
    static constexpr size_t maxRects = 8;

    void add(const Rect &r);
    void addAll(const DamageRegion &o);
    void clear() { rs.clear(); }
    bool empty() const { return rs.empty(); }
    const std::vector<Rect> &rects() const { return rs; }
    Rect bounds() const;

  private:
    std::vector<Rect> rs;
};

} // namespace tpx

#endif
