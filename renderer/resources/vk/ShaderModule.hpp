#pragma once

#include "common/VkCommon.hpp"
#include "VkResourceBase.hpp"
#include "ShaderCompiler.hpp"

namespace X::Backend {

class ShaderModule : public VkResourceBase {
public:
    virtual ~ShaderModule();
    ShaderType GetType() { return type_; }
    std::string GetName() { return shaderName_; }
    vk::ShaderModule GetHandle() { return shader_; }

protected:
    friend class ShaderManager;
    ShaderModule(std::string fileName, ShaderType type);
    ShaderModule(ShaderModule&& other) noexcept
        : type_(other.type_), shaderName_(other.shaderName_), shaderUnique_(std::move(other.shaderUnique_)), shader_(*shaderUnique_) {}

private:
    static std::string ReadShader(std::string fileName);
    static std::string GenerateShaderName(std::string fileName);

private:
    ShaderType type_;
    std::string shaderName_;
    vk::UniqueShaderModule shaderUnique_;
    vk::ShaderModule shader_;
};
    
} // namespace X::Backend
