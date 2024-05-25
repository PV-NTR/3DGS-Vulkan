#pragma once

namespace X {

class UIOverlay {
public:
    bool IsVisible() const { return visible_; }
    void SwitchVisibility();

private:
    bool visible_ = true;
};

} // namespace X