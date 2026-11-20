#ifndef TINY_PIXEL_EXAMPLES_PNG_H
#define TINY_PIXEL_EXAMPLES_PNG_H

#include <string>
#include "tiny-pixel/surface.h"

namespace tpx::png
{
bool write(const Image &img, const std::string &path);
} // namespace tpx::png

#endif
