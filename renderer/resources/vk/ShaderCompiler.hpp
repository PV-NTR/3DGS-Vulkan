#pragma once

#include <unordered_map>
#include <memory>

#include "utils/Singleton.hpp"
#include <glslang/Public/ShaderLang.h>

namespace X::Backend {

enum class ShaderType {
    Vertex = 0,
    TesseControl,
    TesseEval,
    Geometry,
    Fragment,
    Compute,
    // ray tracing
    RayGen,
    AnyHit,
    ClosestHit,
    Miss,
    Intersection,
    Callable,
    // mesh shader
    Task,
    Mesh,
};

class ShaderCompiler : public Singleton<ShaderCompiler> {
public:
    bool CompileShader(const std::string& code, ShaderType type, std::vector<uint32_t>& spirv);

private:
    std::unordered_map<ShaderType, std::unique_ptr<glslang::TShader>> shaders_;
};

} // namespace X::Backend