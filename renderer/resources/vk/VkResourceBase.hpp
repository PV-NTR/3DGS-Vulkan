#pragma once

#include <memory>
#include <vector>
#include "resources/Resource.hpp"

namespace X::Backend {

class VkResourceBase : public Resource {
public:
    VkResourceBase() noexcept = default;
    virtual ~VkResourceBase() noexcept = default;

    VkResourceBase& operator=(VkResourceBase&& other) = delete;

    void DependOn(const std::shared_ptr<VkResourceBase>& dep)
    {
        dependencies_.emplace_back(dep);
    }

protected:
    std::vector<std::shared_ptr<VkResourceBase>> dependencies_;
};
    
} // namespace X::Backend
