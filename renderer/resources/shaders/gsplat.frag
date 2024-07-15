#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in mediump vec4 inColor;
layout (location = 1) in mediump vec2 inPos;

layout (location = 0) out mediump vec4 outColor;

void main()
{
    float A = -dot(inPos, inPos);
    if (A < -4.0f) discard;
    // is lower bound necessary?
    float B = clamp(exp(A) * inColor.a, 0.0f, 1.0f);
    outColor = vec4(B * inColor.rgb, B);
}