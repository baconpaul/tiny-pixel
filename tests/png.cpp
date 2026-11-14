#include "png.h"

#include <filesystem>
#include <vector>

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace tpx::test
{
bool writePng(const Image &img, const std::string &path)
{
    auto parent = std::filesystem::path(path).parent_path();
    if (!parent.empty())
        std::filesystem::create_directories(parent);
    std::vector<unsigned char> rgb(size_t(img.width()) * img.height() * 3);
    size_t o = 0;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
        {
            auto p = img.at(x, y);
            rgb[o++] = (unsigned char)((p >> 16) & 0xFF);
            rgb[o++] = (unsigned char)((p >> 8) & 0xFF);
            rgb[o++] = (unsigned char)(p & 0xFF);
        }
    return stbi_write_png(path.c_str(), img.width(), img.height(), 3, rgb.data(),
                          img.width() * 3) != 0;
}
} // namespace tpx::test
