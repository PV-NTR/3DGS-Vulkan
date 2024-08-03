#pragma once

#include <memory>
#include <vector>
#include "resources/Resource.hpp"

namespace X::Backend {

class VkResourceBase : public Resource {
public:
    VkResourceBase() noexcept = default;
    VkResourceBase(VkResourceBase&& other) noexcept
    {
        dependencies_.swap(other.dependencies_);
    }

    virtual ~VkResourceBase() noexcept = default;

    void DependOn(const std::shared_ptr<VkResourceBase>& dep)
    {
        dependencies_.emplace_back(dep);
    }

protected:
    VkResourceBase& operator=(VkResourceBase&& other) = delete;
    std::vector<std::shared_ptr<VkResourceBase>> dependencies_;
};
    
} // namespace X::Backend
