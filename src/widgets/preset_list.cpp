#include "to_pixels/widgets/preset_list.h"

#include <algorithm>

#include "to_pixels/canvas.h"
#include "to_pixels/font.h"
#include "to_pixels/root.h"

namespace tpx
{

namespace
{
constexpr int barW = 3, scrollW = 3, textPad = 12;
} // namespace

PresetList::PresetList() { setFocusable(true); }

int PresetList::rowHeight()
{
    auto *r = root();
    if (r && r->theme().font)
        return r->theme().font->lineHeight() + 8;
    return 24;
}

int PresetList::visibleRows() { return std::max(1, bounds().h / rowHeight()); }

void PresetList::setItems(std::vector<std::string> items)
{
    names = std::move(items);
    if (names.empty())
        sel = -1;
    else
        sel = std::clamp(sel, 0, int(names.size()) - 1);
    scrollIntoView();
    repaint();
}

void PresetList::setSelected(int index)
{
    const int old = sel, oldTop = top;
    sel = names.empty() ? -1 : std::clamp(index, 0, int(names.size()) - 1);
    scrollIntoView();
    if (top != oldTop)
        repaint();
    else if (sel != old)
    {
        repaintRow(old);
        repaintRow(sel);
    }
}

void PresetList::scrollIntoView()
{
    const int rows = visibleRows();
    if (sel >= 0)
    {
        if (sel < top)
            top = sel;
        if (sel >= top + rows)
            top = sel - rows + 1;
    }
    top = std::clamp(top, 0, std::max(0, int(names.size()) - rows));
}

void PresetList::repaintRow(int index)
{
    auto *r = root();
    if (!r || index < 0 || index < top || index >= top + visibleRows())
        return;
    const int rh = rowHeight();
    const int y0 = (bounds().h - visibleRows() * rh) / 2;
    const Rect b = boundsInRoot();
    r->markDirty({b.x, b.y + y0 + (index - top) * rh, b.w, rh});
}

Rect PresetList::focusAnchor()
{
    const int rh = rowHeight(), rows = visibleRows();
    if (sel < top || sel >= top + rows)
        return Component::focusAnchor();
    const int y0 = (bounds().h - rows * rh) / 2;
    return {0, y0 + (sel - top) * rh, bounds().w, rh};
}

bool PresetList::moveTo(int index)
{
    if (names.empty())
        return false;
    index = std::clamp(index, 0, int(names.size()) - 1);
    if (index == sel)
        return false;
    setSelected(index);
    if (onSelect)
        onSelect(sel);
    return true;
}

bool PresetList::onInput(const InputEvent &e)
{
    switch (e.key)
    {
    case Input::Up:
        return e.pressed && moveTo(sel - 1);
    case Input::Down:
        return e.pressed && moveTo(sel + 1);
    case Input::Adjust:
        // positive is the next preset, as bacon beats does it
        moveTo(sel + e.delta * (e.coarse ? visibleRows() : 1));
        return true;
    case Input::Edit:
        if (e.pressed && sel >= 0 && onChoose)
            onChoose(sel);
        return true;
    default:
        return false;
    }
}

void PresetList::paint(Canvas &c)
{
    auto *r = root();
    if (!r)
        return;
    const Theme &t = r->theme();
    const int w = bounds().w, h = bounds().h;
    c.fillRoundedRect({0, 0, float(w), float(h)}, t.radius, t.bg1);

    const int rh = rowHeight(), rows = visibleRows();
    const int y0 = (h - rows * rh) / 2;
    const bool scrolls = int(names.size()) > rows;
    const int rowW = w - 8 - (scrolls ? scrollW + 4 : 0);

    for (int i = 0; i < rows && top + i < int(names.size()); ++i)
    {
        const int idx = top + i;
        const Rect row{4, y0 + i * rh, rowW, rh};
        const bool isSel = idx == sel;
        if (isSel)
        {
            c.fillRoundedRect({float(row.x), float(row.y) + 2, float(row.w), float(rh) - 4}, 4,
                              t.bg2);
            c.fillRoundedRect({float(row.x) + 2, float(row.y) + 6, float(barW), float(rh) - 12},
                              1.5f, t.accL);
        }
        if (t.font)
            c.drawText(*t.font, Rect{row.x + textPad, row.y, row.w - textPad - 4, rh},
                       names[size_t(idx)], isSel ? t.txt0 : t.txt1, Align::Left);
    }

    if (scrolls)
    {
        const float tx = float(w - 4 - scrollW), ty = 6, th = float(h - 12);
        const float n = float(names.size());
        const float thumbH = std::max(12.f, th * float(rows) / n);
        const float thumbY = ty + (th - thumbH) * float(top) / std::max(1.f, n - float(rows));
        c.fillRoundedRect({tx, ty, float(scrollW), th}, 1.5f, t.bg2);
        c.fillRoundedRect({tx, thumbY, float(scrollW), thumbH}, 1.5f, t.line);
    }
}

} // namespace tpx
