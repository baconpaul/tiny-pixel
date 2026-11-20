#ifndef TINY_PIXEL_WIDGETS_VALUE_CONTROL_H
#define TINY_PIXEL_WIDGETS_VALUE_CONTROL_H

#include <functional>
#include <string>

#include "../component.h"

namespace tpx
{

// a normalised 0..1 value edited with Adjust; the base of Knob and Slider
class ValueControl : public Component
{
  public:
    ValueControl();

    // from the host: clamps, NaN becomes the default, never fires onChange
    void setValue(float v01);
    float value() const { return val; }
    void setDefault(float v01);
    float defaultValue() const { return def; }
    void setSteps(float fine, float coarse);
    void setLabel(std::string l);
    const std::string &label() const { return lbl; }
    void setBipolar(bool b);
    bool bipolar() const { return bip; }
    std::string valueText() const;

    std::function<void(float)> onChange;
    std::function<std::string(float)> formatValue;

    bool onInput(const InputEvent &e) override;

  private:
    float val{0}, def{0}, fineStep{0.01f}, coarseStep{0.1f};
    bool bip{false};
    std::string lbl;
};

} // namespace tpx

#endif
