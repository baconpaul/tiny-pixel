#ifndef TINY_PIXEL_REPEAT_H
#define TINY_PIXEL_REPEAT_H

#include "theme.h"

namespace tpx
{

// the extra steps a held button earns: none until a delay, then faster the longer it is held
struct Repeat
{
    void start(const Theme &t)
    {
        on = true;
        held = 0;
        in = t.repeatDelay;
        slow = t.repeatSlow;
        fast = t.repeatFast;
        fastAfter = t.repeatFastAfter;
    }
    void stop() { on = false; }
    bool active() const { return on; }

    int tick(float dt)
    {
        if (!on)
            return 0;
        held += dt;
        in -= dt;
        int steps = 0;
        while (in <= 0)
        {
            ++steps;
            in += held > fastAfter ? fast : slow;
        }
        return steps;
    }

  private:
    bool on{false};
    float in{0}, held{0}, slow{0.09f}, fast{0.03f}, fastAfter{1.0f};
};

} // namespace tpx

#endif
