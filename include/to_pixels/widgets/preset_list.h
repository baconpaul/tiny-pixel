#ifndef TO_PIXELS_WIDGETS_PRESET_LIST_H
#define TO_PIXELS_WIDGETS_PRESET_LIST_H

#include <functional>
#include <string>
#include <vector>

#include "../component.h"

namespace tpx
{

// a scrolling list; up/down browse and leave at the ends, Edit chooses
class PresetList : public Component
{
  public:
    PresetList();

    void setItems(std::vector<std::string> items);
    const std::vector<std::string> &items() const { return names; }
    // from the app: clamps, scrolls into view, never fires onSelect
    void setSelected(int index);
    int selected() const { return sel; }
    int scrollTop() const { return top; }
    int visibleRows();
    int rowHeight();

    std::function<void(int)> onSelect;
    std::function<void(int)> onChoose;

    bool onInput(const InputEvent &e) override;
    void paint(Canvas &c) override;
    Rect focusAnchor() override;

  private:
    bool moveTo(int index);
    void scrollIntoView();
    void repaintRow(int index);

    std::vector<std::string> names;
    int sel{-1}, top{0};
};

} // namespace tpx

#endif
