#ifndef TO_PIXELS_EXAMPLES_PNG_H
#define TO_PIXELS_EXAMPLES_PNG_H

#include <string>
#include "to_pixels/surface.h"

namespace tpx::png
{
bool write(const Image &img, const std::string &path);
} // namespace tpx::png

#endif
