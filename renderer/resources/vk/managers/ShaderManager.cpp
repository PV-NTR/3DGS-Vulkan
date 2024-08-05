#include "ShaderManager.hpp"

#include <fstream>
#include <sstream>

#include "resources/vk/VkContext.hpp"
#include "common/LogCommon.hpp"

namespace X::Backend {

ShaderManager::ShaderManager()
{
#ifdef HOST_ANDROID
    androidAssetManager_ = g_androidAppCtx->activity->assetManager;
#endif
}

std::string ShaderManager::ReadShader(std::string fileName)
{
#ifdef HOST_WIN32
    std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);
    std::stringstream ss;

    if (is.is_open()) {
        size_t size = is.tellg();
        is.seekg(0, std::ios::beg);
        ss << is.rdbuf();
        is.close();
        std::string shaderCode = ss.str();

        assert(size > 0);

        return shaderCode;
    } else {
        XLOGE("ReadShader failed, open file failed, fileName: %s!", fileName.c_str());
        return "";
    }
#elif defined HOST_ANDROID
    // Load shader from compressed asset
    AAsset* asset = AAssetManager_open(androidAssetManager_, fileName.c_str(), AASSET_MODE_STREAMING);
    assert(asset);
    size_t size = AAsset_getLength(asset);
    assert(size > 0);

    char* shaderCode = new char[size + 1];
    AAsset_read(asset, shaderCode, size);
    AAsset_close(asset);
    shaderCode[size] = '\0';
    std::stringstream ss(shaderCode);
    delete[] shaderCode;
    return ss.str();
#endif
}

std::string ShaderManager::GenerateShaderName(std::string fileName)
{
    size_t split = fileName.find_last_of("/\\");
    std::string shaderName = fileName.substr(split + 1);
    split = shaderName.find_first_of(".");
    shaderName = shaderName.substr(0, split) + "_" + shaderName.substr(split + 1);
    return shaderName;
}

std::shared_ptr<ShaderModule> ShaderManager::AddShaderModule(std::string fileName, ShaderType shaderType)
{
    std::string shaderName = GenerateShaderName(fileName);
    if (freeResources_.find(shaderName) == freeResources_.end()) {
        std::string shaderCode = ReadShader(fileName);
        freeResources_[shaderName] =
            std::unique_ptr<ShaderModule>(new ShaderModule(shaderName, shaderCode, shaderType));
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
    freeResources_[shaderModulePtr->GetName()] =
        std::unique_ptr<ShaderModule>(new ShaderModule(std::move(*shaderModulePtr)));
}

} // namespace X::Backend
