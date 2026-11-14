#include "to_pixels/surface.h"

#include <algorithm>

namespace tpx
{

Image::Image(int width, int height, uint32_t fill)
    : px(size_t(std::max(width, 0)) * size_t(std::max(height, 0)), fill), w(std::max(width, 0)),
      h(std::max(height, 0))
{
}

Surface Image::surface() { return {px.data(), w, h, w * 4}; }

} // namespace tpx
