#ifndef TINY_PIXEL_WIDGETS_LABEL_H
#define TINY_PIXEL_WIDGETS_LABEL_H

#include <string>

#include "../canvas.h"
#include "../component.h"

namespace tpx
{

class Label : public Component
{
  public:
    enum class Style
    {
        Normal,
        Bold,
        Small,
        Value
    };
    enum class Tone
    {
        Primary,
        Secondary,
        Dim
    };

    void setText(std::string t);
    const std::string &text() const { return txt; }
    void setStyle(Style s);
    void setTone(Tone t);
    void setAlign(Align a);
    void paint(Canvas &c) override;

  private:
    std::string txt;
    Style style{Style::Normal};
    Tone tone{Tone::Primary};
    Align align{Align::Left};
};

} // namespace tpx

#endif
