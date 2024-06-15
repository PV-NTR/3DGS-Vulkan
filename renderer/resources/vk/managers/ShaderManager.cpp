#include "ShaderManager.hpp"

#include "resources/vk/VkContext.hpp"

namespace X::Backend {
    
std::shared_ptr<ShaderModule> ShaderManager::AddShaderModule(std::string fileName, ShaderType shaderType)
{
    std::string shaderName = ShaderModule::GenerateShaderName(fileName);
    if (freeResources_.find(shaderName) == freeResources_.end()) {
        freeResources_[shaderName] = std::unique_ptr<ShaderModule>(new ShaderModule(fileName, shaderType));
    }
    std::shared_ptr<ShaderModule> ret(freeResources_[shaderName].release(), [this](ShaderModule* shaderModulePtr) {
            this->Recycle(shaderModulePtr);
        });
    freeResources_.erase(fileName);
    return ret;
}

void ShaderManager::Recycle(ShaderModule* shaderModulePtr)
{
    while (freeResources_.size() >= CACHE_SIZE) {
        auto frontFreeResource = resourceQueue_.front();
        resourceQueue_.pop();
        freeResources_.erase(frontFreeResource);
    }
    resourceQueue_.push(shaderModulePtr->GetName());
    freeResources_[shaderModulePtr->GetName()] = std::unique_ptr<ShaderModule>(new ShaderModule(std::move(*shaderModulePtr)));
}

} // namespace X::Backend
