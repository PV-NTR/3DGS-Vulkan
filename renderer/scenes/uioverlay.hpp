#pragma once

#include "imgui.h"

namespace X {

class UIOverlay {
public:
    bool IsVisible() const { return visible_; }
    void SwitchVisibility();
    bool Updated() const;

private:
    bool visible_ = true;
};

} // namespace X