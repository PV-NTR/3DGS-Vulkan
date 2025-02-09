#version 450
#extension GL_KHR_vulkan_glsl : enable

// TODO: compress data
struct SplatData {
    float pos[3];
    float n[3];
    float color[3];
    float shs[45];
    float opacity;
    float scale[3];
    float rot[4];
};

layout (binding = 0, std430) restrict readonly buffer SSBO_SPLATDATA {
    SplatData splatData[];
};

layout (binding = 1, std140) uniform UBO_OBJECT_INDEX_PREFIXSUM {
    uvec4 prefixSums[8];
};

layout (binding = 2) uniform UBO_MODEL {
    mat4 models[32];
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


const float SH_C0 = 0.28209479177387814;
const float SH_C1 = 0.48860251190291992;

const float SH_C2[5] = float[](
    1.09254843059207907, -1.09254843059207907, 0.31539156525252000,
    -1.09254843059207907, 0.54627421529603953
);

const float SH_C3[7] = float[](
    -0.59004358992664351, 2.89061144264055405, -0.45704579946446573, 0.37317633259011539,
    -0.45704579946446573, 1.445305721320277027, -0.59004358992664351
);

mat3 QuatToMat3(vec4 q)
{
    mat3 result;
    result[0][0] = 1 - 2 * q[1] * q[1] - 2 * q[2] * q[2];
    result[0][1] = 2 * q[0] * q[1] - 2 * q[2] * q[3];
    result[0][2] = 2 * q[0] * q[2] + 2 * q[1] * q[3];
    result[1][0] = 2 * q[0] * q[1] + 2 * q[2] * q[3];
    result[1][1] = 1 - 2 * q[0] * q[0] - 2 * q[2] * q[2];
    result[1][2] = 2 * q[1] * q[2] - 2 * q[0] * q[3];
    result[2][0] = 2 * q[0] * q[2] - 2 * q[1] * q[3];
    result[2][1] = 2 * q[1] * q[2] + 2 * q[0] * q[3];
    result[2][2] = 1 - 2 * q[0] * q[0] - 2 * q[1] * q[1];
    return result;
}

vec3 CalcColor(vec3 color, float[45] shsFloats, float x, float y, float z)
{
    vec3 shs[15];
    for (int i = 0; i < 15; i++) {
        shs[i] = vec3(shsFloats[3 * i], shsFloats[3 * i + 1], shsFloats[3 * i + 2]);
    }
    vec3 color0 = color * SH_C0;
    vec3 color1 = SH_C1 * (-shs[0] * y + shs[1] * z + shs[2] * x);

    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, yz = y * z, zx = z * x;
    vec3 color2 = SH_C2[0] * shs[3] * xy + SH_C2[1] * shs[4] * yz + SH_C2[2] * shs[5] * (2 * zz - xx - yy) +
        SH_C2[3] * shs[6] * zx + SH_C2[4] * shs[7] * (xx - yy);
    
    vec3 color3 = -SH_C3[0] * shs[8] * y * (3 * xx - yy) +
        SH_C3[1] * shs[9] * x * yz -
        SH_C3[2] * shs[10] * y * (5 * zz - 1) +
        SH_C3[3] * shs[11] * z * (2 * zz - 3 * xx - 3 * yy) +
        SH_C3[4] * shs[12] * x * (5 * zz - 1) +
        SH_C3[5] * shs[13] * z * (xx - yy) +
        SH_C3[6] * shs[14] * x * (xx - 3 * yy);
    return clamp(color0 + color1 + color2 + color3, vec3(0.0f), vec3(1.0f));
}

void main()
{
    // TODO: fix errors
    SplatData splat = splatData[gl_InstanceIndex];

    uint objectID = 0;
    for (int i = 0; i < 32; i++) {
        if (prefixSums[objectID / 4][object % 4] >= gl_InstanceIndex) {
            objectID = i;
            break;
        }
    }

    vec3 center = vec3(splat.pos[0], splat.pos[1], splat.pos[2]);
    mat4 mv = view * models[objectID];
    vec4 cam = mv * vec4(center, 1.0f);
    vec4 pos2d = proj * cam;

    float clip = 1.2 * pos2d.w;
    if (pos2d.z < -pos2d.w || pos2d.z > pos2d.w || pos2d.x < -clip || pos2d.x > clip || pos2d.y < -clip || pos2d.y > clip) {
        gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
        return;
    }
    mat3 J = mat3(
        focal.x / cam.z, 0.f, -(focal.x * cam.x) / (cam.z * cam.z),
        0.f, -focal.y / cam.z, (focal.y * cam.y) / (cam.z * cam.z),
        0.f, 0.f, 0.f
    );
    mat3 T = transpose(mat3(mv)) * J;
    mat3 rot = QuatToMat3(vec4(splat.rot[0], splat.rot[1], splat.rot[2], splat.rot[3]));
    mat3 scale = mat3(1.0f);
    scale[0][0] = splat.scale[0];
    scale[1][1] = splat.scale[1];
    scale[2][2] = splat.scale[2];
    mat3 cov = scale * rot;
    mat3 cov2d = transpose(T) * transpose(cov) * cov * T;

    cov2d[0][0] += 0.3f;
    cov2d[1][1] += 0.3f;

    float mid = (cov2d[0][0] + cov2d[1][1]) / 2.0;
    float radius = length(vec2((cov2d[0][0] - cov2d[1][1]) / 2.0, cov2d[0][1]));
    float lambda1 = mid + radius, lambda2 = mid - radius;

    if (lambda2 < 0.0) return;
    vec2 diagonalVector = normalize(vec2(cov2d[0][1], lambda1 - cov2d[0][0]));
    vec2 majorAxis = min(sqrt(2.0 * lambda1), 1024.0) * diagonalVector;
    vec2 minorAxis = min(sqrt(2.0 * lambda2), 1024.0) * vec2(diagonalVector.y, -diagonalVector.x);

    vec3 dir = normalize(center - inverse(view)[3].xyz);
    vec3 color = CalcColor(vec3(splat.color[0], splat.color[1], splat.color[2]), splat.shs, dir.x, dir.y, dir.z);
    outColor = vec4(color, splat.opacity);
    outPos = inPos;

    gl_Position = vec4(
        pos2d.xy / pos2d.w + inPos.x * majorAxis / viewport + inPos.y * minorAxis / viewport,
        0.0f, 1.0f);
}