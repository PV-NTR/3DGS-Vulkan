#pragma once

#include <memory>
#include <unordered_map>
#include <queue>

#include "resources/vk/ShaderModule.hpp"

#ifdef HOST_ANDROID
#include <android/asset_manager.h>
#include "AndroidMain.hpp"
#endif

namespace X::Backend {

class ShaderManager {
public:
    ShaderManager();
    std::shared_ptr<ShaderModule> AddShaderModule(std::string fileName, ShaderType shaderType);

private:
    void Recycle(ShaderModule* shaderModulePtr);

private:
    std::string ReadShader(std::string fileName);
    std::string GenerateShaderName(std::string fileName);

private:
    static inline constexpr uint32_t CACHE_SIZE = 256;

    std::unordered_map<std::string, std::unique_ptr<ShaderModule>> freeResources_;
    std::queue<std::string> resourceQueue_;
#ifdef HOST_ANDROID
    AAssetManager* androidAssetManager_ = nullptr;
#endif
};
    
} // namespace X::Backend
