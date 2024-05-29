#include "UIOverlay.hpp"

namespace X {

void UIOverlay::SwitchVisibility()
{
    visible_ = !visible_;
}

bool UIOverlay::Updated() const
{
    return false;
}

} // namespace X
