#include "to_pixels/focus.h"

#include <algorithm>
#include <cmath>

namespace tpx
{

int findNeighbour(const Rect &from, const std::vector<Rect> &candidates, Input dir)
{
    if (dir != Input::Up && dir != Input::Down && dir != Input::Left && dir != Input::Right)
        return -1;
    const bool vertical = dir == Input::Up || dir == Input::Down;
    const float fx = float(from.x) + float(from.w) / 2, fy = float(from.y) + float(from.h) / 2;
    auto gap = [](float a0, float a1, float b0, float b1) {
        return std::max(0.f, std::max(a0, b0) - std::min(a1, b1));
    };

    int best = -1;
    float bestScore = 0;
    for (int i = 0; i < int(candidates.size()); ++i)
    {
        const Rect &b = candidates[size_t(i)];
        if (b == from)
            continue;
        const float ddx = float(b.x) + float(b.w) / 2 - fx;
        const float ddy = float(b.y) + float(b.h) / 2 - fy;
        const float along = dir == Input::Right  ? ddx
                            : dir == Input::Left ? -ddx
                            : dir == Input::Down ? ddy
                                                 : -ddy;
        if (along <= 0)
            continue;
        // overlapping boxes are in line, however wide, and beat the rest; centres break ties
        const float off =
            vertical ? gap(float(from.x), float(from.right()), float(b.x), float(b.right()))
                     : gap(float(from.y), float(from.bottom()), float(b.y), float(b.bottom()));
        const float score =
            along + 4 * off + 0.1f * std::fabs(vertical ? ddx : ddy) + (off > 0 ? 10000.f : 0.f);
        if (best < 0 || score < bestScore)
        {
            best = i;
            bestScore = score;
        }
    }
    return best;
}

} // namespace tpx
