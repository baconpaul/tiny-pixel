#ifndef TINY_PIXEL_WIDGETS_SLIDER_H
#define TINY_PIXEL_WIDGETS_SLIDER_H

#include "value_control.h"

namespace tpx
{

class Slider : public ValueControl
{
  public:
    enum class Orientation
    {
        Horizontal,
        Vertical
    };

    void setOrientation(Orientation o);
    Orientation orientation() const { return orient; }
    void paint(Canvas &c) override;

  private:
    Orientation orient{Orientation::Horizontal};
};

} // namespace tpx

#endif
