#ifndef TO_PIXELS_TESTS_PNG_H
#define TO_PIXELS_TESTS_PNG_H

#include <string>
#include "to_pixels/surface.h"

namespace tpx::test
{
bool writePng(const Image &img, const std::string &path);
} // namespace tpx::test

#endif
