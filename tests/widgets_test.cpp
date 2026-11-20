#include "catch2.hpp"

#include <limits>
#include <string>
#include <vector>

#include "tiny-pixel/tiny-pixel.h"
#include "tiny-pixel/default_font.h"

TEST_CASE("adjust moves the value, clamps, and reports", "[widgets]")
{
    tpx::Knob k;
    std::vector<float> seen;
    k.onChange = [&](float v) { seen.push_back(v); };
    k.setValue(0.5f);
    REQUIRE(seen.empty());
    REQUIRE(k.onInput({tpx::Input::Adjust, true, 1, false}));
    REQUIRE(k.value() == Approx(0.51f));
    k.onInput({tpx::Input::Adjust, true, 1, true});
    REQUIRE(k.value() == Approx(0.61f));
    k.setValue(1.0f);
    seen.clear();
    REQUIRE(k.onInput({tpx::Input::Adjust, true, 1, true}));
    REQUIRE(seen.empty()); // already at the top
}

TEST_CASE("host values are clamped and NaN falls back to default", "[widgets]")
{
    tpx::Slider s;
    int calls = 0;
    s.onChange = [&](float) { ++calls; };
    s.setDefault(0.25f);
    s.setValue(3.0f);
    REQUIRE(s.value() == 1.0f);
    s.setValue(-1.0f);
    REQUIRE(s.value() == 0.0f);
    s.setValue(std::numeric_limits<float>::quiet_NaN());
    REQUIRE(s.value() == 0.25f);
    REQUIRE(calls == 0);
}

TEST_CASE("value text uses the formatter", "[widgets]")
{
    tpx::Knob k;
    k.setValue(0.42f);
    REQUIRE(k.valueText() == "42%");
    k.formatValue = [](float v) { return std::to_string(int(v * 1000)) + " Hz"; };
    REQUIRE(k.valueText() == "420 Hz");
}

TEST_CASE("a knob step repaints only the knob", "[widgets]")
{
    tpx::Root root(200, 100);
    auto d = tpx::fonts::jetbrainsMonoRegular();
    tpx::Font small(d.data(), d.size(), 12);
    root.theme().fontSmall = &small;
    tpx::Knob k1, k2;
    k1.setBounds({0, 0, 80, 100});
    k2.setBounds({100, 0, 80, 100});
    root.addChild(&k1);
    root.addChild(&k2);
    tpx::Image img(200, 100);
    root.render(img.surface(), 0);
    root.setFocus(&k1);
    root.render(img.surface(), 1);
    img.surface().row(50)[150] = 0xFF123456; // inside k2: a partial render must keep it
    img.surface().row(50)[190] = 0xFF123456; // bare root background
    root.input({tpx::Input::Edit, true});
    root.input({tpx::Input::Right, true});
    REQUIRE(root.needsFrame());
    root.render(img.surface(), 1);
    REQUIRE(img.at(150, 50) == 0xFF123456u);
    REQUIRE(img.at(190, 50) == 0xFF123456u);
}

TEST_CASE("labels are not focusable and repaint on change", "[widgets]")
{
    tpx::Root root(100, 40);
    tpx::Label l;
    l.setBounds({0, 0, 100, 40});
    root.addChild(&l);
    tpx::Image img(100, 40);
    root.render(img.surface(), 0);
    REQUIRE_FALSE(l.focusable());
    l.setText("hello");
    REQUIRE(root.needsFrame());
    REQUIRE(l.text() == "hello");
}
