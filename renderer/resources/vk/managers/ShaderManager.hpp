#pragma once

#include <memory>
#include <unordered_map>
#include <queue>

#include "resources/vk/ShaderModule.hpp"

namespace X::Backend {

class ShaderManager {
public:
    ShaderManager() = default;
    std::shared_ptr<ShaderModule> AddShaderModule(std::string fileName, ShaderType shaderType);

private:
    void Recycle(ShaderModule* shaderModulePtr);

private:
    static inline constexpr uint32_t CACHE_SIZE = 256;

    std::unordered_map<std::string, std::unique_ptr<ShaderModule>> freeResources_;
    std::queue<std::string> resourceQueue_;
};
    
} // namespace X::Backend
