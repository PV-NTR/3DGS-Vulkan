#pragma once

#include "utils/Utils.hpp"

namespace X {

class Resource {
public:
    Resource() = default;
    virtual ~Resource() = default;

protected:
    const uint32_t resourceID_ = NewID();
};

} // namespace X
