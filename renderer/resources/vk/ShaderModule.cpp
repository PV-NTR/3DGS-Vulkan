#include "ShaderModule.hpp"

#include <fstream>
#include <sstream>

#include "common/LogCommon.hpp"
#include "VkContext.hpp"

namespace X::Backend {

std::string ShaderModule::ReadShader(std::string fileName)
{
    std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);
    std::stringstream ss;

    if (is.is_open())
    {
        size_t size = is.tellg();
        is.seekg(0, std::ios::beg);
        ss << is.rdbuf();
        is.close();
        std::string shaderCode = ss.str();

        assert(size > 0);

        return shaderCode;
    } else {
        XLOGE("ReadShader failed, open file failed!");
        return "";
    }
}

ShaderModule::ShaderModule(std::string fileName, ShaderType type)
    : type_(type)
{
    std::string shaderCode = ReadShader(fileName);
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
