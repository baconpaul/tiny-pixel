#ifndef TINY_PIXEL_INPUT_H
#define TINY_PIXEL_INPUT_H

namespace tpx
{

// the host maps its buttons onto these; Adjust is made by Root from Edit + d-pad or Encoder
enum class Input
{
    Up,
    Down,
    Left,
    Right,
    Edit,
    Back,
    PagePrev,
    PageNext,
    Encoder,
    Adjust
};

struct InputEvent
{
    Input key;
    bool pressed{true};
    int delta{0};
    bool coarse{false};
};

} // namespace tpx

#endif
