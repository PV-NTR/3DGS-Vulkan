#version 450
#extension GL_KHR_vulkan_glsl : enable

struct Preprocessed {
    vec4 pos2d;
    vec4 color;
    vec2 majorAxis;
    vec2 minorAxis;
    uint modelID;
    bool visible;
    bool valid;
};

layout (set = 1, binding = 0, std430) restrict readonly buffer SSBO_PREPROCESSED {
    Preprocessed preprocessed[];
};

layout (set = 1, binding = 1, std430) restrict readonly buffer SSBO_SORTED {
    uint sorted[];
};

layout (binding = 2) uniform UBO_MODEL {
    mat4 models[];
};

layout (binding = 3) uniform UBO_CAMERA {
    mat4 view;
    mat4 proj;
    vec2 focal;
};

layout (binding = 4) uniform UBO_SCREEN {
    vec2 viewport;
};

layout (location = 0) in vec2 inPos;

layout (location = 0) out mediump vec4 outColor;
layout (location = 1) out mediump vec2 outPos;

void main()
{
    Preprocessed data = preprocessed[sorted[gl_InstanceIndex]];

    if (!data.visible) {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }

    if (!data.valid) {
        return;
    }

    outColor = data.color;
    outPos = inPos;

    gl_Position = vec4(
        data.pos2d.xy / data.pos2d.w + inPos.x * data.majorAxis / viewport + inPos.y * data.minorAxis / viewport,
        0.0f, 1.0f);
}