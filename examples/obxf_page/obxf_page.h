#ifndef TINY_PIXEL_EXAMPLES_OBXF_PAGE_H
#define TINY_PIXEL_EXAMPLES_OBXF_PAGE_H

#include <array>
#include <vector>

#include "tiny-pixel/tiny-pixel.h"

// an OB-Xf-like small-devices screen over a fake parameter set
class ObxfPage
{
  public:
    static constexpr int width = 640, height = 480;

    enum Param
    {
        Cutoff,
        Resonance,
        EnvAmount,
        OscMix,
        Attack,
        Decay,
        Sustain,
        Release,
        Volume,
        Detune,
        numParams
    };

    ObxfPage();

    tpx::Root &root() { return rt; }
    tpx::Knob &knob(int i) { return knobs[size_t(i)]; }
    tpx::PresetList &presets() { return list; }
    tpx::Slider &volume() { return vol; }
    tpx::Slider &detune() { return det; }
    const std::vector<float> &params() const { return vals; }

  private:
    // declared in this order so fonts outlive the root and the root outlives the widgets
    tpx::Font font, fontBold, fontSmall, fontValue;
    tpx::Root rt{width, height};
    tpx::Label title, presetName;
    tpx::PresetList list;
    std::array<tpx::Knob, 8> knobs;
    tpx::Slider vol, det;
    std::vector<float> vals;
};

#endif
