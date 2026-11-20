#include "obxf_page.h"

#include <cmath>
#include <cstdio>
#include <string>

#include "tiny-pixel/default_font.h"

namespace
{
tpx::Font regular(float px)
{
    auto d = tpx::fonts::jetbrainsMonoRegular();
    return tpx::Font(d.data(), d.size(), px);
}

tpx::Font bold(float px)
{
    auto d = tpx::fonts::jetbrainsMonoBold();
    return tpx::Font(d.data(), d.size(), px);
}

template <class... A> std::string fmt(const char *f, A... a)
{
    char buf[64];
    std::snprintf(buf, sizeof(buf), f, a...);
    return buf;
}

std::string hz(float v)
{
    const float f = 20.f * std::pow(1000.f, v);
    return f >= 1000.f ? fmt("%.1f kHz", double(f / 1000.f)) : fmt("%.0f Hz", double(f));
}

std::string seconds(float v)
{
    const float s = 0.001f * std::pow(10000.f, v);
    return s >= 1.f ? fmt("%.1f s", double(s)) : fmt("%.0f ms", double(s * 1000.f));
}

std::string signedPercent(float v)
{
    const int p = int(std::lround((v * 2.f - 1.f) * 100.f));
    return p == 0 ? std::string("0%") : fmt("%+d%%", p);
}

std::string mix(float v)
{
    const int b = int(std::lround(v * 100.f));
    return fmt("A %d / %d B", 100 - b, b);
}

std::string decibels(float v)
{
    if (v <= 0.f)
        return "-inf dB";
    return fmt("%+.1f dB", double(20.f * std::log10(v * v * 2.f)));
}

std::string cents(float v)
{
    const int c = int(std::lround((v * 2.f - 1.f) * 50.f));
    return c == 0 ? std::string("0 ct") : fmt("%+d ct", c);
}

const char *presetNames[] = {
    "Bass | Mono Brass",   "Bass | Rubber Low",  "Bass | Sub Pulse",    "Bass | Growl Saw",
    "Brass | Jump Stab",   "Brass | Soft Horns", "Brass | Big Section", "Keys | Glass EP",
    "Keys | Velvet Clav",  "Keys | Toy Organ",   "Lead | Sync Scream",  "Lead | Thin Whistle",
    "Lead | Detuned Saws", "Lead | Mono Glide",  "Pad | Glass Choir",   "Pad | Warm Strings",
    "Pad | Slow Bloom",    "Pad | Night Drift",  "Pad | Wide Unison",   "Pluck | Short Pick",
    "Pluck | Kalimba",     "Pluck | Harp Bits",  "Poly | Classic 8",    "Poly | Bright Stack",
    "Poly | Dark Chords",  "FX | Sweep Up",      "FX | Noise Wind",     "FX | Laser Drop",
    "Seq | Pulse Train",   "Seq | Arp Fodder",
};
} // namespace

ObxfPage::ObxfPage()
    : font(regular(15)), fontBold(bold(18)), fontSmall(regular(12)), fontValue(bold(13)),
      vals(numParams, 0.f)
{
    auto &t = rt.theme();
    t.font = &font;
    t.fontBold = &fontBold;
    t.fontSmall = &fontSmall;
    t.fontValue = &fontValue;

    title.setText("OB-Xf");
    title.setStyle(tpx::Label::Style::Bold);
    title.setBounds({16, 6, 80, 36});
    presetName.setTone(tpx::Label::Tone::Secondary);
    presetName.setBounds({100, 6, 524, 36});
    rt.addChild(&title);
    rt.addChild(&presetName);

    std::vector<std::string> names(std::begin(presetNames), std::end(presetNames));
    presetName.setText(names.front());
    list.setItems(names);
    list.setSelected(0);
    list.setBounds({12, 52, 200, 416});
    list.onChoose = [this](int i) { presetName.setText(list.items()[size_t(i)]); };
    rt.addChild(&list);

    struct Spec
    {
        const char *name;
        float init;
        bool bipolar;
        std::string (*format)(float);
    };
    const Spec specs[8] = {
        {"Cutoff", 0.62f, false, hz},
        {"Resonance", 0.3f, false, nullptr},
        {"Env Amt", 0.68f, true, signedPercent},
        {"Osc Mix", 0.4f, false, mix},
        {"Attack", 0.1f, false, seconds},
        {"Decay", 0.45f, false, seconds},
        {"Sustain", 0.7f, false, nullptr},
        {"Release", 0.5f, false, seconds},
    };
    for (int i = 0; i < 8; ++i)
    {
        auto &k = knobs[size_t(i)];
        const auto &s = specs[i];
        k.setLabel(s.name);
        k.setBipolar(s.bipolar);
        k.setDefault(s.init);
        k.setValue(s.init);
        if (s.format)
            k.formatValue = s.format;
        vals[size_t(i)] = s.init;
        k.onChange = [this, i](float v) { vals[size_t(i)] = v; };
        k.setBounds({228 + (i % 4) * 100, 52 + (i / 4) * 140, 96, 136});
        rt.addChild(&k);
    }

    det.setOrientation(tpx::Slider::Orientation::Vertical);
    det.setLabel("Detune");
    det.setBipolar(true);
    det.setDefault(0.5f);
    det.setValue(0.56f);
    det.formatValue = cents;
    vals[Detune] = det.value();
    det.onChange = [this](float v) { vals[Detune] = v; };
    det.setBounds({228, 344, 96, 124});
    rt.addChild(&det);

    vol.setLabel("Volume");
    vol.setDefault(0.7f);
    vol.setValue(0.7f);
    vol.formatValue = decibels;
    vals[Volume] = vol.value();
    vol.onChange = [this](float v) { vals[Volume] = v; };
    vol.setBounds({336, 380, 292, 60});
    rt.addChild(&vol);
}
