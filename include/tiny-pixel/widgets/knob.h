#ifndef TINY_PIXEL_WIDGETS_KNOB_H
#define TINY_PIXEL_WIDGETS_KNOB_H

#include "value_control.h"

namespace tpx
{

// a 270 degree knob with its label and value beneath
class Knob : public ValueControl
{
  public:
    void paint(Canvas &c) override;
};

} // namespace tpx

#endif
