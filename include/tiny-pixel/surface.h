#ifndef TINY_PIXEL_SURFACE_H
#define TINY_PIXEL_SURFACE_H

#include <cstdint>
#include <vector>

#include "geometry.h"

namespace tpx
{

// a view of someone else's XRGB8888 pixels
struct Surface
{
    uint32_t *pixels{nullptr};
    int width{0}, height{0}, strideBytes{0};

    uint32_t *row(int y) const
    {
        return reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(pixels) +
                                            size_t(y) * size_t(strideBytes));
    }
    Rect bounds() const { return {0, 0, width, height}; }
};

class Image
{
  public:
    Image(int w, int h, uint32_t fill = 0xFF000000);

    Surface surface();
    int width() const { return w; }
    int height() const { return h; }
    uint32_t at(int x, int y) const { return px[size_t(y) * size_t(w) + size_t(x)]; }

  private:
    std::vector<uint32_t> px;
    int w{0}, h{0};
};

} // namespace tpx

#endif
