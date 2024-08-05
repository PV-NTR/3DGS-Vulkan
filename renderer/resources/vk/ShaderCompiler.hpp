#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>

#include <glslang/Public/ShaderLang.h>
#include "resources/Enums.hpp"
#include "utils/Singleton.hpp"

namespace X::Backend {

class ShaderCompiler : public Singleton<ShaderCompiler> {
public:
    ShaderCompiler();
    ~ShaderCompiler();
    bool CompileShader(const std::string& code, ShaderType type, std::vector<uint32_t>& spirv);

private:
    std::unordered_map<ShaderType, std::unique_ptr<glslang::TShader>> shaders_;
    std::mutex mutex_;
};

} // namespace X::Backend