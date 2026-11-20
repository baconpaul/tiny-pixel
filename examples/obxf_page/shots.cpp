#include <cstdio>
#include <string>

#include "obxf_page.h"
#include "png.h"

// drives the page through a scripted session, writing a png after each step from partial redraws
int main(int argc, char **argv)
{
    const std::string out = argc > 1 ? argv[1] : "build/shots/obxf";
    ObxfPage page;
    auto &root = page.root();
    tpx::Image img(ObxfPage::width, ObxfPage::height);
    int n = 0;
    bool ok = true;

    auto shot = [&](const char *name) {
        root.render(img.surface(), n == 0 ? 0 : 1);
        char file[256];
        std::snprintf(file, sizeof(file), "%s/%02d-%s.png", out.c_str(), n++, name);
        ok &= tpx::png::write(img, file);
        std::printf("%s\n", file);
    };
    auto press = [&](tpx::Input k, int times = 1) {
        for (int i = 0; i < times; ++i)
        {
            root.input({k, true});
            root.input({k, false});
        }
    };
    auto hold = [&](bool on) { root.input({tpx::Input::Edit, on}); };

    shot("start");
    press(tpx::Input::Down);
    shot("list-focused");
    press(tpx::Input::Down, 3);
    shot("list-down-3");
    press(tpx::Input::Edit);
    shot("preset-chosen");
    press(tpx::Input::Right);
    shot("cutoff-focused");
    hold(true);
    press(tpx::Input::Right, 5);
    shot("cutoff-fine-up");
    press(tpx::Input::Up, 2);
    shot("cutoff-coarse-up");
    hold(false);
    press(tpx::Input::Down);
    shot("attack-focused");
    press(tpx::Input::Up);
    press(tpx::Input::Right, 2);
    shot("env-focused");
    hold(true);
    press(tpx::Input::Left, 4);
    press(tpx::Input::Down, 2);
    shot("env-negative");
    hold(false);
    press(tpx::Input::Down, 2);
    shot("sliders");
    press(tpx::Input::Left);
    press(tpx::Input::Left);
    shot("list-scroll-start");
    press(tpx::Input::Down, 20);
    shot("list-scrolled");
    return ok ? 0 : 1;
}
