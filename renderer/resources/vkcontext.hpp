#pragma once

#include <memory>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

class VkContext {
public:
    VkContext();
    bool IsReady();

private:
};