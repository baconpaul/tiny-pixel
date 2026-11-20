#ifndef TINY_PIXEL_FOCUS_H
#define TINY_PIXEL_FOCUS_H

#include <vector>

#include "geometry.h"
#include "input.h"

namespace tpx
{

// the best candidate from `from` in direction dir, or -1; candidates equal to from are skipped
int findNeighbour(const Rect &from, const std::vector<Rect> &candidates, Input dir);

} // namespace tpx

#endif
