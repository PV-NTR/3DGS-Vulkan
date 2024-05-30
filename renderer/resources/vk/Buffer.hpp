#pragma once

#include "VkResourceBase.hpp"
#include "vma/VmaImage.hpp"
#include "resources/Enums.hpp"

namespace X::Backend {

class BufferManager;

struct BufferInfo {
    uint64_t size_;
    BufferType type_;
};

class Buffer : public VkResourceBase<Buffer>{
private:
    using Parent = VkResourceBase<Buffer>;

public:
    virtual ~Buffer() noexcept = default;
};

} // namespace X::Backend