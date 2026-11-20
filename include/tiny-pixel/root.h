#ifndef TINY_PIXEL_ROOT_H
#define TINY_PIXEL_ROOT_H

#include <array>
#include <cstdint>
#include <functional>

#include "component.h"
#include "damage.h"
#include "repeat.h"
#include "surface.h"
#include "theme.h"

namespace tpx
{

// the top of a tree: owns theme, focus and the dirty region, and renders on request
class Root : public Component
{
  public:
    static constexpr int historyDepth = 4;

    Root(int w, int h);
    ~Root() override;

    void setSize(int w, int h);
    Theme &theme() { return th; }
    const Theme &theme() const { return th; }
    // repaints everything; editing theme() directly does not
    void setTheme(const Theme &t);

    void markDirty(const Rect &rootRect);
    bool needsFrame() const { return forceFull || !dirty.empty(); }
    // fired when the first dirt appears after a render: the place to call request_frame()
    std::function<void()> onNeedsFrame;
    // input nothing in the focus chain consumed
    std::function<bool(const InputEvent &)> onUnhandled;

    // bufferAge as pixels/1 gives it: 0 unknown, n the buffer holds the frame from n renders ago
    bool render(Surface s, uint32_t bufferAge);

    // see the design doc: d-pad moves focus, Edit held turns it into Adjust, with key repeat
    bool input(const InputEvent &e);
    // drives key repeat; true if a repeat step was consumed
    bool tick(double dtSeconds);

    Component *focused() const { return focus; }
    void setFocus(Component *c);
    // drops focus if c is or contains it, without calling into c
    void forgetFocus(const Component *c);

  protected:
    Root *asRoot() override { return this; }

  private:
    void paintTree(Component &c, Canvas &g);
    void needFrame(bool before);
    bool sendToFocus(const InputEvent &e);
    bool moveFocus(Input dir);
    void collectFocusable(Component &c, std::vector<Component *> &out);

    bool editHeld{false};
    Repeat repeat;
    Input repeatDir{Input::Up};

    Theme th;
    DamageRegion dirty;
    std::array<DamageRegion, historyDepth> history;
    int historyCount{0};
    bool forceFull{true};
    Component *focus{nullptr};
};

} // namespace tpx

#endif
