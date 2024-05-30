#pragma once

#include <memory>
#include <vector>
#include "resources/Resource.hpp"

namespace X::Backend {

template <typename VkResource>
class VkResourceBase : public Resource {
public:
    VkResourceBase() noexcept = default;
    virtual ~VkResourceBase() noexcept = default;

    VkResourceBase& operator=(VkResourceBase&& other) = delete;

    void DependOn(const std::shared_ptr<VkResourceBase>& dep)
    {
        dependencies_.emplace_back(dep);
    }

private:
    std::vector<std::shared_ptr<VkResourceBase>> dependencies_;
};
    
} // namespace X::Backend
