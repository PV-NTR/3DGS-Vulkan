#pragma once

#include "resources/Resource.hpp"

namespace X::Backend {

struct NoManager {};
class VkResourceBase : public Resource {};

template <typename VkObjUnique, typename Manager = NoManager>
class VkResource : public VkObjUnique, public VkResourceBase {
public:
    static constexpr bool HAS_MANAGER = !std::is_same_v<Manager, NoManager>;

public:
    VkResource() noexcept = default;
    virtual ~VkResource() noexcept
    {
        Recycle();
        VkObjUnique::reset();
    }

    template <typename... Args, std::enable_if_t<std::is_constructible_v<VkObjUnique, Args...>, bool> = true>
    explicit VkResource(Args&&... args) noexcept : VkObjUnique(std::forward<Args>(args)...) {}

    VkResource& operator=(VkResource&& oth) = delete;

    void DependOn(const std::shared_ptr<VkResourceBase>& dep)
    {
        dependencies_.emplace_back(dep);
    }

    void WeakDependOn(const std::shared_ptr<VkResourceBase>& dep)
    {
        weakDependencies_.emplace_back(dep);
    }

private:
    void Recycle() noexcept
    {
        if constexpr (HAS_MANAGER) {
            if (manager_) {
                weakDependencies_.clear();
                auto tmp = manager_;
                manager_ = nullptr;
                tmp->Recycle(std::move);
            }
        }
    }

private:
    std::vector<std::shared_ptr<VkResourceBase>> dependencies_;
    std::vector<std::shared_ptr<VkResourceBase>> weakDependencies_;

    std::shared_ptr<Manager> manager_ = nullptr;
};
    
} // namespace X::Backend
