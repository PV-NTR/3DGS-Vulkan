#include "ShaderModule.hpp"

#include "common/LogCommon.hpp"
#include "VkContext.hpp"
#include "managers/ShaderManager.hpp"

namespace X::Backend {

ShaderModule::ShaderModule(const std::string& shaderName, const std::string& shaderCode, ShaderType type)
    : shaderName_(shaderName), type_(type)
{
    if (shaderCode.empty()) {
        return;
    }

    std::vector<uint32_t> dst;
    if (!ShaderCompiler::GetInstance().CompileShader(shaderCode, type, dst)) {
        XLOGE("CompileShader failed!");
        return;
    }
    vk::ShaderModuleCreateInfo shaderCI {};
    shaderCI.setCode(dst);

    auto [ret, shaderUnique] = VkContext::GetInstance().GetDevice().createShaderModuleUnique(shaderCI);
    if (ret != vk::Result::eSuccess) {
        XLOGE("CreateShaderModule failed, errCode: %d", ret);
        return;
    }
    shaderUnique_.swap(shaderUnique);
    shader_ = *shaderUnique_;
}

ShaderModule::~ShaderModule()
{
    if (shader_) {
        VkContext::GetInstance().GetDevice().destroyShaderModule(shader_);
    }
}

} // namespace X::Backend
