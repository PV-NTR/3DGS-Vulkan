#pragma once

#include "utils/Utils.hpp"

namespace X {

class Resource {
public:
    Resource() = default;
    ~Resource() = default;

private:
    uint32_t resourceID_ = NewID();
};

} // namespace X
