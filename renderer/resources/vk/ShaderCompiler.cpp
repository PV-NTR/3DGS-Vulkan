#include "ShaderCompiler.hpp"

#include "common/LogCommon.hpp"

#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>

namespace X::Backend {

bool ShaderCompiler::CompileShader(const std::string& code, ShaderType type, std::vector<uint32_t>& spirv)
{
    if (shaders_.find(type) == shaders_.end()) {
        shaders_.insert({ type, std::unique_ptr<glslang::TShader>(new glslang::TShader(static_cast<EShLanguage>(type))) });
    }
    auto& shader = shaders_[type];
    const char* shaderStrings[1] = { code.c_str() };
    shader->setStrings(shaderStrings, 1);
    EShMessages messages = EShMsgDefault;
    if (!shader->parse(GetResources(), 100, false, messages)) {
        XLOGE("CompileShader parse failed, glslang info: %s\n, debug info: %s\n", shader->getInfoLog(), shader->getInfoDebugLog());
        return false;
    }

    glslang::TProgram program;
    program.addShader(&(*shader));
    if (!program.link(messages)) {
        XLOGE("CompileShader link failed, glslang info: %s\n, debug info: %s\n", shader->getInfoLog(), shader->getInfoDebugLog());
        return false;
    }

    if (program.getIntermediate(static_cast<EShLanguage>(type))) {
        glslang::GlslangToSpv(*program.getIntermediate(static_cast<EShLanguage>(type)), spirv);
    } else {
        XLOGE("CompileShader unknown error, glslang info: %s\n, debug info: %s\n", shader->getInfoLog(), shader->getInfoDebugLog());
        return false;
    }
    return true;
}
    
} // namespace X::Backend