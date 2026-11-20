#include "to_pixels/root.h"

#include <algorithm>

#include "to_pixels/canvas.h"
#include "to_pixels/focus.h"

namespace tpx
{

Root::Root(int w, int h) { bnds = {0, 0, w, h}; }

Root::~Root()
{
    for (auto *k : kids)
        k->par = nullptr;
    kids.clear();
    focus = nullptr;
}

void Root::needFrame(bool before)
{
    if (!before && needsFrame() && onNeedsFrame)
        onNeedsFrame();
}

void Root::setSize(int w, int h)
{
    const bool before = needsFrame();
    bnds = {0, 0, w, h};
    forceFull = true;
    needFrame(before);
}

void Root::setTheme(const Theme &t)
{
    const bool before = needsFrame();
    th = t;
    forceFull = true;
    needFrame(before);
}

void Root::markDirty(const Rect &rootRect)
{
    const Rect r = rootRect.intersect(bnds);
    if (r.empty())
        return;
    const bool before = needsFrame();
    dirty.add(r);
    needFrame(before);
}

bool Root::render(Surface s, uint32_t bufferAge)
{
    const bool full = forceFull || bufferAge == 0 || bufferAge > uint32_t(historyDepth) ||
                      int(bufferAge) - 1 > historyCount;
    DamageRegion region;
    if (full)
        region.add(bnds);
    else
    {
        region = dirty;
        for (uint32_t i = 0; i + 1 < bufferAge; ++i)
            region.addAll(history[i]);
    }
    if (region.empty())
        return false;

    Canvas g(s);
    for (const auto &r : region.rects())
    {
        g.save();
        g.clipTo(r);
        g.fillRect(r, th.bg0);
        paint(g);
        for (auto *k : kids)
            paintTree(*k, g);
        g.restore();
    }

    for (int i = historyDepth - 1; i > 0; --i)
        history[size_t(i)] = history[size_t(i - 1)];
    if (full)
    {
        history[0].clear();
        history[0].add(bnds);
    }
    else
        history[0] = dirty;
    historyCount = std::min(historyCount + 1, historyDepth);
    dirty.clear();
    forceFull = false;
    return true;
}

void Root::paintTree(Component &c, Canvas &g)
{
    if (!c.vis || !c.bnds.intersects(g.clip()))
        return;
    g.save();
    g.translate(c.bnds.x, c.bnds.y);
    g.clipTo({0, 0, c.bnds.w, c.bnds.h});
    c.paint(g);
    for (auto *k : c.kids)
        paintTree(*k, g);
    if (focus == &c)
        c.paintFocus(g);
    g.restore();
}

void Root::setFocus(Component *c)
{
    if (c == focus)
        return;
    if (c)
    {
        if (!c->focusOk || c->root() != this)
            return;
        for (const Component *p = c; p; p = p->par)
            if (!p->vis)
                return;
    }
    auto *old = focus;
    focus = c;
    if (old)
        old->focusChanged(false);
    if (c)
        c->focusChanged(true);
}

void Root::forgetFocus(const Component *c)
{
    for (const Component *f = focus; f; f = f->par)
        if (f == c)
        {
            focus = nullptr;
            return;
        }
}

} // namespace tpx

namespace tpx
{

namespace
{
bool isDirection(Input k)
{
    return k == Input::Up || k == Input::Down || k == Input::Left || k == Input::Right;
}

InputEvent adjustFor(Input dir)
{
    const bool up = dir == Input::Right || dir == Input::Up;
    return {Input::Adjust, true, up ? 1 : -1, dir == Input::Up || dir == Input::Down};
}
} // namespace

bool Root::sendToFocus(const InputEvent &e)
{
    for (Component *c = focus; c; c = c->par)
        if (c->onInput(e))
            return true;
    return false;
}

void Root::collectFocusable(Component &c, std::vector<Component *> &out)
{
    if (!c.vis)
        return;
    if (c.focusOk)
        out.push_back(&c);
    for (auto *k : c.kids)
        collectFocusable(*k, out);
}

bool Root::moveFocus(Input dir)
{
    std::vector<Component *> cs;
    collectFocusable(*this, cs);
    if (cs.empty())
        return false;
    if (!focus)
    {
        setFocus(cs.front());
        return focus != nullptr;
    }
    cs.erase(std::remove(cs.begin(), cs.end(), focus), cs.end());
    std::vector<Rect> rs;
    rs.reserve(cs.size());
    for (auto *c : cs)
        rs.push_back(c->boundsInRoot());
    const Rect at = focus->boundsInRoot();
    const int best = findNeighbour(focus->focusAnchor().translated(at.x, at.y), rs, dir);
    if (best < 0)
        return false;
    setFocus(cs[size_t(best)]);
    return true;
}

bool Root::input(const InputEvent &e)
{
    if (e.key == Input::Edit)
    {
        editHeld = e.pressed;
        if (!e.pressed)
        {
            repeat.stop();
            return focus != nullptr;
        }
        const bool used = sendToFocus(e);
        return used || focus != nullptr || (onUnhandled && onUnhandled(e));
    }

    if (isDirection(e.key))
    {
        if (!e.pressed)
        {
            if (repeat.active() && repeatDir == e.key)
                repeat.stop();
            return false;
        }
        if (editHeld && focus)
        {
            repeat.start(th);
            repeatDir = e.key;
            sendToFocus(adjustFor(e.key));
            return true;
        }
        if (focus && sendToFocus(e))
            return true;
        if (moveFocus(e.key))
            return true;
        return onUnhandled && onUnhandled(e);
    }

    if (e.key == Input::Encoder || e.key == Input::Adjust)
    {
        const InputEvent a{Input::Adjust, true, e.delta, e.key == Input::Adjust && e.coarse};
        return sendToFocus(a) || (onUnhandled && onUnhandled(e));
    }

    if (!e.pressed)
        return false;
    return sendToFocus(e) || (onUnhandled && onUnhandled(e));
}

bool Root::tick(double dtSeconds)
{
    if (!repeat.active() || !editHeld || !focus)
        return false;
    const int steps = repeat.tick(float(dtSeconds));
    bool used = false;
    for (int i = 0; i < steps; ++i)
        used |= sendToFocus(adjustFor(repeatDir));
    return used;
}

} // namespace tpx
