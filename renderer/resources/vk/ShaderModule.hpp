#pragma once

#include "common/VkCommon.hpp"
#include "VkResourceBase.hpp"
#include "ShaderCompiler.hpp"

namespace X::Backend {

class ShaderModule : public VkResourceBase {
public:
    ShaderModule(std::string fileName, ShaderType type);
    virtual ~ShaderModule();
    ShaderType GetType() { return type_; }
    vk::ShaderModule GetHandle() { return shader_; }

private:
    static std::string ReadShader(std::string fileName);

private:
    ShaderType type_;
    vk::UniqueShaderModule shaderUnique_;
    vk::ShaderModule shader_;
};
    
} // namespace X::Backend
