#ifndef TO_PIXELS_COMPONENT_H
#define TO_PIXELS_COMPONENT_H

#include <vector>

#include "geometry.h"
#include "input.h"

namespace tpx
{

class Canvas;
class Root;

// the app owns components, usually as members; parents only point at children
class Component
{
  public:
    Component() = default;
    virtual ~Component();
    Component(const Component &) = delete;
    Component &operator=(const Component &) = delete;

    // in parent coordinates
    void setBounds(const Rect &r);
    const Rect &bounds() const { return bnds; }
    Rect boundsInRoot() const;

    void addChild(Component *c);
    void removeChild(Component *c);
    const std::vector<Component *> &children() const { return kids; }
    Component *parent() const { return par; }
    Root *root();

    void setVisible(bool v);
    bool visible() const { return vis; }
    void setFocusable(bool f);
    bool focusable() const { return focusOk; }
    bool hasFocus();

    void repaint();

    // the canvas is translated to this component and clipped to its bounds
    virtual void paint(Canvas &) {}
    virtual void paintFocus(Canvas &c);
    virtual bool onInput(const InputEvent &) { return false; }
    virtual void focusChanged(bool) { repaint(); }

  protected:
    virtual Root *asRoot() { return nullptr; }

  private:
    friend class Root;

    Rect bnds;
    Component *par{nullptr};
    std::vector<Component *> kids;
    bool vis{true}, focusOk{false};
};

} // namespace tpx

#endif
