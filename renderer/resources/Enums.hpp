#pragma once

namespace X {

enum class ColorType {


};

enum class BufferType {
    Vertex = 0,
    Index,
    Uniform,
    Storage,
    Staging,
};

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

} // namespace X::Backend
