#ifndef TINY_PIXEL_CANVAS_H
#define TINY_PIXEL_CANVAS_H

#include <string_view>
#include <vector>

#include "color.h"
#include "geometry.h"
#include "surface.h"

namespace tpx
{

class Font;

enum class Align
{
    Left,
    Centre,
    Right
};

// angles are radians, 0 at 12 o'clock, positive clockwise on screen
class Canvas
{
  public:
    explicit Canvas(Surface s);

    void save();
    void restore();
    void translate(int dx, int dy);
    void clipTo(const Rect &r);
    Rect clip() const;

    void fillRect(const Rect &r, Color c);
    void fillRoundedRect(const RectF &r, float radius, Color c);
    void fillRoundedRect(const RectF &r, float radius, Color top, Color bottom);
    // the stroke lies inside r
    void strokeRoundedRect(const RectF &r, float radius, float width, Color c);
    void fillCircle(PointF centre, float r, Color c);
    void strokeCircle(PointF centre, float r, float width, Color c);
    void strokeArc(PointF centre, float r, float startRad, float endRad, float width, Color c);
    void line(PointF a, PointF b, float width, Color c);

    void drawText(Font &f, float x, float baselineY, std::string_view utf8, Color c);
    // vertically centred on cap height, clipped to box
    void drawText(Font &f, const Rect &box, std::string_view utf8, Color c, Align align);

  private:
    struct State
    {
        Rect clip;
        int tx{0}, ty{0};
    };

    Surface surf;
    State cur;
    std::vector<State> stack;
};

} // namespace tpx

#endif
