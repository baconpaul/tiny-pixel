#include "to_pixels/component.h"

#include <algorithm>

#include "to_pixels/canvas.h"
#include "to_pixels/root.h"

namespace tpx
{

Component::~Component()
{
    if (par)
        par->removeChild(this);
    for (auto *k : kids)
        k->par = nullptr;
    kids.clear();
}

void Component::setBounds(const Rect &r)
{
    if (r == bnds)
        return;
    repaint();
    bnds = r;
    repaint();
}

Rect Component::boundsInRoot() const
{
    Rect r = bnds;
    for (const Component *p = par; p; p = p->par)
        r = r.translated(p->bnds.x, p->bnds.y);
    return r;
}

void Component::addChild(Component *c)
{
    if (!c || c == this || c->par == this)
        return;
    if (c->par)
        c->par->removeChild(c);
    c->par = this;
    kids.push_back(c);
    c->repaint();
}

void Component::removeChild(Component *c)
{
    auto it = std::find(kids.begin(), kids.end(), c);
    if (it == kids.end())
        return;
    c->repaint();
    if (auto *r = root())
        r->forgetFocus(c);
    kids.erase(it);
    c->par = nullptr;
}

Root *Component::root()
{
    Component *c = this;
    while (c->par)
        c = c->par;
    return c->asRoot();
}

void Component::setVisible(bool v)
{
    if (v == vis)
        return;
    repaint();
    vis = v;
    if (!v)
        if (auto *r = root())
            r->forgetFocus(this);
}

void Component::setFocusable(bool f)
{
    focusOk = f;
    if (!f)
        if (auto *r = root(); r && r->focused() == this)
            r->forgetFocus(this);
}

bool Component::hasFocus()
{
    auto *r = root();
    return r && r->focused() == this;
}

void Component::repaint()
{
    if (auto *r = root())
        r->markDirty(boundsInRoot());
}

void Component::paintFocus(Canvas &c)
{
    auto *r = root();
    if (!r)
        return;
    const auto &t = r->theme();
    c.strokeRoundedRect({0, 0, float(bnds.w), float(bnds.h)}, t.radius, t.focusRingWidth, t.accL);
}

} // namespace tpx
