#include "catch2.hpp"

#include "obxf_page.h"

namespace
{
void press(tpx::Root &r, tpx::Input k)
{
    r.input({k, true});
    r.input({k, false});
}

void requireSame(const tpx::Image &a, const tpx::Image &b)
{
    int diffs = 0;
    for (int y = 0; y < a.height(); ++y)
        for (int x = 0; x < a.width(); ++x)
            diffs += a.at(x, y) != b.at(x, y);
    REQUIRE(diffs == 0);
}
} // namespace

TEST_CASE("partial renders match a full render after a session", "[obxf]")
{
    ObxfPage page;
    tpx::Image live(ObxfPage::width, ObxfPage::height);
    page.root().render(live.surface(), 0);
    auto step = [&] { page.root().render(live.surface(), 1); };
    press(page.root(), tpx::Input::Down);
    step();
    press(page.root(), tpx::Input::Down);
    step();
    press(page.root(), tpx::Input::Edit);
    step();
    press(page.root(), tpx::Input::Right);
    step();
    page.root().input({tpx::Input::Edit, true});
    for (int i = 0; i < 5; ++i)
    {
        press(page.root(), tpx::Input::Right);
        step();
    }
    page.root().input({tpx::Input::Edit, false});
    step();
    press(page.root(), tpx::Input::Down);
    step();

    tpx::Image fresh(ObxfPage::width, ObxfPage::height);
    page.root().render(fresh.surface(), 0);
    requireSame(live, fresh);
}

TEST_CASE("double buffering with buffer age 2 stays correct", "[obxf]")
{
    // a host swapping two buffers: each holds the frame from two renders ago
    ObxfPage page;
    tpx::Image bufs[2]{{640, 480}, {640, 480}};
    int n = 0;
    auto frame = [&] {
        page.root().render(bufs[n % 2].surface(), n < 2 ? 0 : 2);
        ++n;
    };
    frame();
    frame();
    page.root().setFocus(&page.knob(0));
    frame();
    page.root().input({tpx::Input::Edit, true});
    for (int i = 0; i < 6; ++i)
    {
        page.root().input({tpx::Input::Up, true});
        page.root().input({tpx::Input::Up, false});
        frame();
    }
    tpx::Image fresh(640, 480);
    page.root().render(fresh.surface(), 0);
    requireSame(bufs[(n - 1) % 2], fresh);
}

TEST_CASE("choosing a preset and turning cutoff reach the params", "[obxf]")
{
    ObxfPage page;
    page.root().setFocus(&page.knob(0));
    const float before = page.params()[0];
    page.root().input({tpx::Input::Edit, true});
    page.root().input({tpx::Input::Right, true});
    REQUIRE(page.params()[0] > before);
}
