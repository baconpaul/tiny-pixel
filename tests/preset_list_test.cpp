#include "catch2.hpp"

#include <string>
#include <vector>

#include "to_pixels/widgets/preset_list.h"

static std::vector<std::string> names(int n)
{
    std::vector<std::string> v;
    for (int i = 0; i < n; ++i)
        v.push_back("Preset " + std::to_string(i));
    return v;
}

TEST_CASE("up and down move the selection and leave at the ends", "[presets]")
{
    tpx::PresetList p;
    p.setBounds({0, 0, 200, 240}); // 10 rows of 24 px with no font
    p.setItems(names(3));
    p.setSelected(0);
    int picked = -1;
    p.onSelect = [&](int i) { picked = i; };
    REQUIRE_FALSE(p.onInput({tpx::Input::Up}));
    REQUIRE(p.onInput({tpx::Input::Down}));
    REQUIRE(picked == 1);
    p.onInput({tpx::Input::Down});
    REQUIRE_FALSE(p.onInput({tpx::Input::Down}));
    REQUIRE(p.selected() == 2);
}

TEST_CASE("coarse adjust pages and scrolls into view", "[presets]")
{
    tpx::PresetList p;
    p.setBounds({0, 0, 200, 120}); // 5 rows
    p.setItems(names(40));
    p.setSelected(0);
    p.onInput({tpx::Input::Adjust, true, 1, true});
    REQUIRE(p.selected() == 5);
    REQUIRE(p.scrollTop() <= 5);
    REQUIRE(p.scrollTop() + p.visibleRows() > 5);
    p.setSelected(39);
    REQUIRE(p.scrollTop() == 35);
}

TEST_CASE("edit chooses, and an empty list is harmless", "[presets]")
{
    tpx::PresetList p;
    p.setBounds({0, 0, 200, 120});
    int chosen = -2;
    p.onChoose = [&](int i) { chosen = i; };
    p.setItems({});
    REQUIRE(p.selected() == -1);
    p.onInput({tpx::Input::Edit});
    p.onInput({tpx::Input::Down});
    p.onInput({tpx::Input::Adjust, true, 3, true});
    REQUIRE(chosen == -2);
    p.setItems(names(10));
    p.setSelected(8);
    p.setItems(names(4));
    REQUIRE(p.selected() == 3);
    p.onInput({tpx::Input::Edit});
    REQUIRE(chosen == 3);
}

#include "to_pixels/root.h"
#include "to_pixels/widgets/knob.h"

TEST_CASE("leaving the list sideways goes to the control level with the selection", "[presets]")
{
    tpx::Root root(300, 400);
    tpx::PresetList p;
    tpx::Knob top, middle;
    p.setBounds({0, 0, 100, 400});
    top.setBounds({150, 0, 50, 60});
    middle.setBounds({150, 170, 50, 60});
    root.addChild(&p);
    root.addChild(&top);
    root.addChild(&middle);
    p.setItems(names(20));
    p.setSelected(1);
    root.setFocus(&p);
    root.input({tpx::Input::Right});
    REQUIRE(root.focused() == &top);
}
