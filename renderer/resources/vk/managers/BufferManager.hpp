#pragma once

#include <unordered_map>
#include <memory>

#include "resources/vk/Buffer.hpp"

namespace X::Backend {

class BufferManager {
public:
    BufferManager() = default;
    std::shared_ptr<Buffer> RequireBuffer();

private:
    std::unordered_map<uint64_t, std::unique_ptr<Buffer>> freeResources_;
};

} // namespace X::Backend