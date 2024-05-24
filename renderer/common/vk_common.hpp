#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
// #define VULKAN_HPP_ASSERT(EXPR)
#include <vulkan/vulkan.hpp>

#define VMA_STATIC_VULKAN_FUNCTIONS 1
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>