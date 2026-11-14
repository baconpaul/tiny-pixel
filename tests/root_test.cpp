#include "catch2.hpp"

#include "to_pixels/canvas.h"
#include "to_pixels/root.h"

namespace
{
struct Box : tpx::Component
{
    tpx::Color c;
    int paints{0};
    explicit Box(tpx::Color col) : c(col) {}
    void paint(tpx::Canvas &g) override
    {
        ++paints;
        g.fillRect({0, 0, bounds().w, bounds().h}, c);
    }
};
} // namespace

TEST_CASE("first render is full and later ones only redraw dirt", "[root]")
{
    tpx::Root root(100, 100);
    Box a(tpx::Color::rgb(0xFF0000)), b(tpx::Color::rgb(0x00FF00));
    a.setBounds({0, 0, 10, 10});
    b.setBounds({50, 50, 10, 10});
    root.addChild(&a);
    root.addChild(&b);
    tpx::Image img(100, 100);
    REQUIRE(root.render(img.surface(), 1));
    REQUIRE(a.paints == 1);
    REQUIRE(b.paints == 1);
    REQUIRE_FALSE(root.render(img.surface(), 1));
    a.repaint();
    REQUIRE(root.render(img.surface(), 1));
    REQUIRE(a.paints == 2);
    REQUIRE(b.paints == 1);
}

TEST_CASE("buffer age replays recent damage", "[root]")
{
    tpx::Root root(100, 100);
    Box a(tpx::Color::rgb(0xFF0000)), b(tpx::Color::rgb(0x00FF00));
    a.setBounds({0, 0, 10, 10});
    b.setBounds({50, 50, 10, 10});
    root.addChild(&a);
    root.addChild(&b);
    tpx::Image img(100, 100);
    root.render(img.surface(), 0);
    a.repaint();
    root.render(img.surface(), 1);
    b.repaint();
    root.render(img.surface(), 2); // buffer is two frames old: a's damage must replay
    REQUIRE(a.paints == 3);
    REQUIRE(b.paints == 2);
    b.repaint();
    root.render(img.surface(), 9); // older than the history: full
    REQUIRE(a.paints == 4);
}

TEST_CASE("onNeedsFrame fires once per batch", "[root]")
{
    tpx::Root root(100, 100);
    Box a(tpx::Color::rgb(0xFF0000));
    a.setBounds({0, 0, 10, 10});
    root.addChild(&a);
    tpx::Image img(100, 100);
    root.render(img.surface(), 0);
    int asks = 0;
    root.onNeedsFrame = [&] { ++asks; };
    a.repaint();
    a.repaint();
    REQUIRE(asks == 1);
    REQUIRE(root.needsFrame());
    root.render(img.surface(), 1);
    a.repaint();
    REQUIRE(asks == 2);
}

TEST_CASE("pixels outside the dirty region are untouched", "[root]")
{
    tpx::Root root(40, 40);
    Box a(tpx::Color::rgb(0xFF0000));
    a.setBounds({0, 0, 10, 10});
    root.addChild(&a);
    tpx::Image img(40, 40);
    root.render(img.surface(), 0);
    img.surface().row(30)[30] = 0xFF123456; // a marker the next partial render must keep
    a.repaint();
    root.render(img.surface(), 1);
    REQUIRE(img.at(30, 30) == 0xFF123456u);
}

TEST_CASE("children paint clipped to their bounds and in parent coordinates", "[root]")
{
    tpx::Root root(40, 40);
    Box parent(tpx::Color::rgb(0x0000FF)), child(tpx::Color::rgb(0xFF0000));
    parent.setBounds({10, 10, 20, 20});
    child.setBounds({15, 15, 20, 20}); // overflows its parent
    root.addChild(&parent);
    parent.addChild(&child);
    tpx::Image img(40, 40);
    root.render(img.surface(), 0);
    REQUIRE(img.at(26, 26) == 0xFFFF0000u);
    REQUIRE(img.at(31, 31) == root.theme().bg0.xrgb());
}
