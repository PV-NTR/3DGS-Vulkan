#pragma once

#include <unordered_map>
#include <memory>

#include "resources/Enums.hpp"
#include "utils/Singleton.hpp"
#include <glslang/Public/ShaderLang.h>

namespace X::Backend {

class ShaderCompiler : public Singleton<ShaderCompiler> {
public:
    bool CompileShader(const std::string& code, ShaderType type, std::vector<uint32_t>& spirv);

private:
    std::unordered_map<ShaderType, std::unique_ptr<glslang::TShader>> shaders_;
};

} // namespace X::Backend