#include "catch2.hpp"

#include <vector>

#include "to_pixels/focus.h"
#include "to_pixels/root.h"

namespace
{
struct Probe : tpx::Component
{
    std::vector<tpx::InputEvent> got;
    bool consumeRaw{false};
    Probe() { setFocusable(true); }
    bool onInput(const tpx::InputEvent &e) override
    {
        got.push_back(e);
        return e.key == tpx::Input::Adjust || consumeRaw;
    }
};
} // namespace

TEST_CASE("findNeighbour prefers controls in line", "[focus]")
{
    // grid: 0 1 2 / 3 4 5, each 10x10 with 20 px pitch
    std::vector<tpx::Rect> r;
    for (int i = 0; i < 6; ++i)
        r.push_back({(i % 3) * 20, (i / 3) * 20, 10, 10});
    REQUIRE(tpx::findNeighbour(r[0], r, tpx::Input::Right) == 1);
    REQUIRE(tpx::findNeighbour(r[0], r, tpx::Input::Down) == 3);
    REQUIRE(tpx::findNeighbour(r[4], r, tpx::Input::Up) == 1);
    REQUIRE(tpx::findNeighbour(r[2], r, tpx::Input::Right) == -1);
}

TEST_CASE("first direction focuses, then moves", "[focus]")
{
    tpx::Root root(100, 100);
    Probe a, b;
    a.setBounds({0, 0, 10, 10});
    b.setBounds({20, 0, 10, 10});
    root.addChild(&a);
    root.addChild(&b);
    REQUIRE(root.input({tpx::Input::Right}));
    REQUIRE(root.focused() == &a);
    REQUIRE(root.input({tpx::Input::Right}));
    REQUIRE(root.focused() == &b);
    REQUIRE_FALSE(root.input({tpx::Input::Right}));
}

TEST_CASE("edit held turns directions into adjust, with repeat", "[focus]")
{
    tpx::Root root(100, 100);
    Probe a;
    a.setBounds({0, 0, 10, 10});
    root.addChild(&a);
    root.setFocus(&a);
    root.input({tpx::Input::Edit, true});
    root.input({tpx::Input::Up, true});
    REQUIRE(a.got.back().key == tpx::Input::Adjust);
    REQUIRE(a.got.back().delta == 1);
    REQUIRE(a.got.back().coarse);
    auto before = a.got.size();
    root.tick(0.3);
    REQUIRE(a.got.size() == before); // still inside the repeat delay
    root.tick(0.2);
    REQUIRE(a.got.size() > before);
    root.input({tpx::Input::Up, false});
    before = a.got.size();
    root.tick(1.0);
    REQUIRE(a.got.size() == before);
}

TEST_CASE("unconsumed input bubbles to the parent", "[focus]")
{
    tpx::Root root(100, 100);
    Probe outer, inner;
    outer.consumeRaw = true;
    outer.setBounds({0, 0, 50, 50});
    inner.setBounds({0, 0, 10, 10});
    root.addChild(&outer);
    outer.addChild(&inner);
    root.setFocus(&inner);
    REQUIRE(root.input({tpx::Input::Back}));
    REQUIRE(outer.got.back().key == tpx::Input::Back);
}

TEST_CASE("destroying the focused component clears focus", "[focus]")
{
    tpx::Root root(100, 100);
    {
        Probe a;
        a.setBounds({0, 0, 10, 10});
        root.addChild(&a);
        root.setFocus(&a);
    }
    REQUIRE(root.focused() == nullptr);
    REQUIRE(root.children().empty());
    tpx::Image img(100, 100);
    REQUIRE(root.render(img.surface(), 0));
}
