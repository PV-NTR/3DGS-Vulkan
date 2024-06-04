#pragma once

#include "common/VkCommon.hpp"
#include "VkResourceBase.hpp"

namespace X::Backend {

class DescriptorSet : public VkResourceBase {
public:
    virtual ~DescriptorSet() = default;

private:

};
    
} // namespace X::Backend
