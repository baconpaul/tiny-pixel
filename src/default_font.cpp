#include "to_pixels/default_font.h"

extern const unsigned char tpx_font_jbm_regular[];
extern const unsigned int tpx_font_jbm_regular_size;
extern const unsigned char tpx_font_jbm_bold[];
extern const unsigned int tpx_font_jbm_bold_size;

namespace tpx::fonts
{
std::span<const unsigned char> jetbrainsMonoRegular()
{
    return {tpx_font_jbm_regular, tpx_font_jbm_regular_size};
}

std::span<const unsigned char> jetbrainsMonoBold()
{
    return {tpx_font_jbm_bold, tpx_font_jbm_bold_size};
}
} // namespace tpx::fonts
