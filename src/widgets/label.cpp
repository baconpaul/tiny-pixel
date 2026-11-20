#include "to_pixels/widgets/label.h"

#include "to_pixels/font.h"
#include "to_pixels/root.h"

namespace tpx
{

void Label::setText(std::string t)
{
    if (t == txt)
        return;
    txt = std::move(t);
    repaint();
}

void Label::setStyle(Style s)
{
    style = s;
    repaint();
}

void Label::setTone(Tone t)
{
    tone = t;
    repaint();
}

void Label::setAlign(Align a)
{
    align = a;
    repaint();
}

void Label::paint(Canvas &c)
{
    auto *r = root();
    if (!r)
        return;
    const Theme &t = r->theme();
    Font *f = style == Style::Bold    ? t.fontBold
              : style == Style::Small ? t.fontSmall
              : style == Style::Value ? t.fontValue
                                      : t.font;
    if (!f)
        f = t.font;
    if (!f)
        return;
    const Color col = tone == Tone::Primary ? t.txt0 : tone == Tone::Secondary ? t.txt1 : t.txt2;
    c.drawText(*f, Rect{0, 0, bounds().w, bounds().h}, txt, col, align);
}

} // namespace tpx
