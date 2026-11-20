#include "tiny-pixel/widgets/value_control.h"

#include <algorithm>
#include <cmath>

namespace tpx
{

ValueControl::ValueControl() { setFocusable(true); }

void ValueControl::setValue(float v01)
{
    const float v = std::isnan(v01) ? def : std::clamp(v01, 0.f, 1.f);
    if (v == val)
        return;
    val = v;
    repaint();
}

void ValueControl::setDefault(float v01)
{
    if (!std::isnan(v01))
        def = std::clamp(v01, 0.f, 1.f);
}

void ValueControl::setSteps(float fine, float coarse)
{
    fineStep = fine;
    coarseStep = coarse;
}

void ValueControl::setLabel(std::string l)
{
    lbl = std::move(l);
    repaint();
}

void ValueControl::setBipolar(bool b)
{
    bip = b;
    repaint();
}

std::string ValueControl::valueText() const
{
    if (formatValue)
        return formatValue(val);
    return std::to_string(int(std::lround(val * 100))) + "%";
}

bool ValueControl::onInput(const InputEvent &e)
{
    if (e.key != Input::Adjust)
        return false;
    const float v = std::clamp(val + float(e.delta) * (e.coarse ? coarseStep : fineStep), 0.f, 1.f);
    if (v != val)
    {
        val = v;
        repaint();
        if (onChange)
            onChange(val);
    }
    // consumed even at the limits, so editing never moves focus
    return true;
}

} // namespace tpx
